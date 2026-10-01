#!/usr/bin/env bash
# Run from the repository root. All generation/build mutations stay in ci-work.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
repo_root=$PWD
output="$repo_root/ci-output"
work="$repo_root/ci-work"
project="$work/firmwaredsp"
ioc="$project/firmwaredsp.ioc"
config=${BUILD_CONFIG:-Debug}
mkdir -p "$output"
fail() { echo "ERROR: $*" >&2; exit 1; }
case "$config" in Debug|Release) ;; *) fail "BUILD_CONFIG must be Debug or Release";; esac

generation_tools() {
  : "${CUBEMX:?Set CUBEMX to the CubeMX executable}"
  : "${FW_REPOSITORY:?Set FW_REPOSITORY to the firmware package repository}"
  [[ -x "$CUBEMX" ]] || fail "CubeMX is missing or inaccessible: $CUBEMX (agent user: $(id -un))"
  CUBEMX=$(realpath "$CUBEMX")
  command -v xvfb-run >/dev/null || fail "Install xvfb and xauth on the agent"
  command -v xauth >/dev/null || fail "Install xauth on the agent"
  command -v timeout >/dev/null || fail "GNU timeout is required"
  command -v flock >/dev/null || fail "flock is required"
  command -v python3 >/dev/null || fail "Python 3 is required"
  # Derive the exact package from the IOC, never install/download in a build.
  package_name=$(python3 - "$repo_root/firmwaredsp/firmwaredsp.ioc" <<'PY'
import re, sys
from pathlib import Path
text = Path(sys.argv[1]).read_text()
m = re.search(r"^ProjectManager.FirmwarePackage=STM32Cube FW_(\w+) V([\d.]+)$", text, re.M)
if not m:
    raise SystemExit("Cannot determine the IOC firmware package")
print(f"STM32Cube_FW_{m[1]}_V{m[2]}")
PY
)
  package_path=$(realpath -m "$FW_REPOSITORY/$package_name")
  [[ -r "$package_path/package.xml" ]] || fail "Install $package_name where this agent can read it: $package_path"
}

build_tools() {
  : "${CUBEIDE_HOME:?Set CUBEIDE_HOME to a current H5E5-capable CubeIDE installation}"
  headless="$CUBEIDE_HOME/headless-build.sh"
  [[ -x "$headless" ]] || fail "Current CubeIDE headless builder missing: $headless. Server CubeIDE 1.11 cannot build H5E5."
  # Use the compiler shipped with the selected IDE, not an arbitrary PATH compiler.
  compiler=$(find "$CUBEIDE_HOME/plugins" -type f -path '*/tools/bin/arm-none-eabi-gcc' -print | sort -V | tail -n 1)
  [[ -n "$compiler" && -x "$compiler" ]] || fail "No bundled ARM GCC in $CUBEIDE_HOME"
  gnu_bin=$(dirname "$compiler")
  major=$("$compiler" -dumpversion | cut -d. -f1)
  [[ "$major" =~ ^[0-9]+$ && "$major" -ge 11 ]] || fail "ARM GCC $major is too old for the generated READONLY linker sections. Upgrade CubeIDE; do not edit linker scripts."
  for tool in objcopy size readelf; do
    [[ -x "$gnu_bin/arm-none-eabi-$tool" ]] || fail "Missing ARM $tool"
  done
  export PATH="$gnu_bin:$PATH"
}

case "${1:-}" in
verify)
  {
    id
    generation_tools
    build_tools
    echo "CubeMX executable: $CUBEMX"
    echo "IOC CubeMX version: $(sed -n 's/^MxCube.Version=//p' firmwaredsp/firmwaredsp.ioc)"
    echo "Firmware package: $package_path"
    echo "CubeIDE: $CUBEIDE_HOME"
    "$compiler" --version
  } 2>&1 | tee "$output/prerequisites.log"
  ;;
generate)
  generation_tools
  # Fixed paths beneath this checkout; never operate on the user's source tree.
  rm -rf "$work"
  mkdir -p "$work"
  cp -a "$repo_root/firmwaredsp" "$project"
  rm -rf "$project/Debug" "$project/Release"
  # Pin the requested package and avoid a migration/download prompt in CI.
  python3 - "$ioc" <<'PY'
from pathlib import Path
import sys
p = Path(sys.argv[1])
text = p.read_text()
text = text.replace("ProjectManager.LastFirmware=true", "ProjectManager.LastFirmware=false")
text = text.replace("ProjectManager.AskForMigrate=true", "ProjectManager.AskForMigrate=false")
p.write_text(text)
PY
  cp "$ioc" "$output/input.ioc"
  cat > "$output/cubemx.script" <<EOF
config load "$ioc"
project generateunderroot 1
project toolchain STM32CubeIDE
project setCustomFWPath "$package_path"
project generate
exit_mx
EOF
  touch "$work/generation.started"
  # Serialize CubeMX across jobs using this same account/cache.
  exec 9>"${TMPDIR:-/tmp}/guitaramp-cubemx-$(id -u).lock"
  flock -w 60 9 || fail "CubeMX is busy; retry this build"
  (
    cd "$(dirname "$CUBEMX")"
    timeout --signal=TERM --kill-after=15s 540s xvfb-run -a "$CUBEMX" -q "$output/cubemx.script"
  ) 2>&1 | tee "$output/cubemx.log"
  flock -u 9
  if grep -Eq '^[[:space:]]*KO[[:space:]]*$|IP not ready for code generation|Exception in thread|Error generating project' "$output/cubemx.log"; then
    fail "CubeMX validation/generation failed; see cubemx.log"
  fi
  # CubeMX may return zero on failure, and preserves timestamps of unchanged
  # source files. Require logged regeneration of this exact source path plus
  # newly generated full-project metadata, not an old ELF or nested project.
  grep -Fq "Generated code: $project/Core/Src/main.c" "$output/cubemx.log" ||
    fail "CubeMX did not regenerate the source path being built"
  [[ -s "$project/Core/Src/main.c" && -s "$project/.project" ]] ||
    fail "Generated source/project missing"
  [[ -s "$project/.cproject" && "$project/.cproject" -nt "$work/generation.started" ]] ||
    fail "CubeMX did not regenerate full CubeIDE metadata"
  [[ -r "$project/Drivers/STM32H5xx_HAL_Driver/Inc/stm32h5xx_hal.h" ]] ||
    fail "CubeMX did not provide the H5 driver library"
  cp "$ioc" "$output/generated.ioc"
  echo "Generated project verified: $project" | tee "$output/generation.txt"
  ;;
build)
  build_tools
  [[ -s "$output/generation.txt" ]] || fail "Run generation first"
  [[ -s "$project/.cproject" ]] || fail "Generated CubeIDE project missing"
  rm -rf "$work/eclipse-workspace" "$project/$config"
  touch "$work/build.started"
  timeout --signal=TERM --kill-after=15s 840s "$headless" \
    -data "$work/eclipse-workspace" -import "$project" \
    -no-indexer -printErrorMarkers -cleanBuild "firmwaredsp/$config" \
    2>&1 | tee "$output/cubeide-build.log"
  elf="$project/$config/firmwaredsp.elf"
  [[ -s "$elf" && "$elf" -nt "$work/build.started" ]] || fail "No fresh firmwaredsp.elf; headless build failed"
  grep -q -- '-mcpu=cortex-m33' "$output/cubeide-build.log" ||
    fail "Build did not select Cortex-M33; upgrade to a CubeIDE version supporting STM32H5E5"
  ;;
package)
  build_tools
  elf="$project/$config/firmwaredsp.elf"
  [[ -s "$elf" && "$elf" -nt "$work/build.started" ]] || fail "No freshly built ELF"
  artifacts="$output/firmware"
  mkdir -p "$artifacts"
  cp "$elf" "$artifacts/firmwaredsp.elf"
  [[ ! -f "$project/$config/firmwaredsp.map" ]] || cp "$project/$config/firmwaredsp.map" "$artifacts/"
  "$gnu_bin/arm-none-eabi-objcopy" -O ihex "$elf" "$artifacts/firmwaredsp.hex"
  "$gnu_bin/arm-none-eabi-objcopy" -O binary "$elf" "$artifacts/firmwaredsp.bin"
  "$gnu_bin/arm-none-eabi-size" "$elf" | tee "$output/firmware-size.txt"
  "$gnu_bin/arm-none-eabi-readelf" -h "$elf" > "$output/elf-header.txt"
  (cd "$artifacts"; sha256sum firmwaredsp.elf firmwaredsp.hex firmwaredsp.bin > SHA256SUMS)
  ;;
*) fail "Usage: bash ci/firmware.sh {verify|generate|build|package}";;
esac

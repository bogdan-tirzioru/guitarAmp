# guitarAmp Jenkins build

The root Jenkinsfile runs checkout → prerequisite checks → CubeMX full-project
generation → CubeIDE compile/link → artifact packaging. Debug is the default;
Release is selectable. Successful builds archive ELF, HEX, BIN, an optional map,
and SHA-256 checksums. Logs, the input/generated IOC, the CubeMX script, and the source
commit are retained on failed builds too.

## Agent prerequisites

Use a Linux Jenkins agent with:
- CubeMX matching the IOC (currently 6.18.1), Xvfb, xauth, Python 3 and GNU coreutils.
- STM32CubeH5 V1.7.0 installed before the build.
- A current STM32CubeIDE supporting STM32H5E5, with its bundled ARM GCC 11 or later.
  The server's existing CubeIDE 1.11 / GCC 10.3 is unsupported. The pipeline fails
  explicitly instead of changing the generated linker script or adding CPU flags.

The selected agent account must be able to execute CubeMX/CubeIDE and read the
firmware repository, and needs a writable home for their configuration/cache.
The current server runs Jenkins as `jenkins`; it cannot traverse
`/home/ghita` (mode 750). Install the tools/packages in a shared location such as
`/opt/st`, or use a deliberately configured build agent with access. Do not run
Jenkins as root or make the entire home public. Initialize CubeMX once under the
agent account, using the exact package; package/license installation is outside
the build. The pipeline does not download packages or accept licenses.

Configure these Jenkins job parameters for the selected agent:

| Parameter | Meaning |
| --- | --- |
| GIT_REF | Branch pattern or tag, initially `*/feature/jenkins-generation-build`; use `*/main` after merge |
| BUILD_CONFIG | Debug or Release |
| CUBEMX | Absolute path to the standalone STM32CubeMX executable |
| CUBEIDE_HOME | Current IDE install directory containing `headless-build.sh` and `plugins/` |
| FW_REPOSITORY | Parent of `STM32Cube_FW_H5_V1.7.0` |

The default CubeIDE path is `/opt/st/stm32cubeide`; create that stable symlink to
your current installation, or supply its versioned path. CubeMX/package defaults
reflect the current user's installations and must be changed for the Jenkins
service account. `agent any` matches the existing BA1/BA2 jobs; restrict it to a
tool-equipped node label if additional agents are introduced.

## Create the job

Create a **Pipeline** job named **guitarAmp-build**. Choose **Pipeline script from
SCM**, Git, repository `https://github.com/bogdan-tirzioru/guitarAmp.git`, credential
`github-bogdan-read`, branch `*/feature/jenkins-generation-build`, and script path
`Jenkinsfile`. Define the initial GIT_REF parameter with the same branch.
After review/merge, change both the SCM branch and GIT_REF to `*/main`.

`ci/jenkins-job.xml` is an equivalent initial job definition based on the existing
BA1/BA2 jobs. An authenticated Jenkins administrator can import it with the
Jenkins CLI `create-job guitarAmp-build < ci/jenkins-job.xml`, or configure the UI.
The branch remains a feature branch until approved for merge.

## Generation and build guarantees

The source tree is copied to `ci-work/firmwaredsp`, preserving existing USER CODE.
CubeMX loads the copied IOC and uses `project generateunderroot 1` so output stays
with that IOC. The pipeline verifies
logged regeneration of `Core/Src/main.c` and fresh `.cproject` metadata in that
same directory before
importing it into a fresh Eclipse workspace. Existing Debug/Release outputs are
removed before compilation. A missing or stale ELF fails the build even if
CubeIDE returns success. The build log must contain `-mcpu=cortex-m33`.

Only the working IOC copy disables automatic firmware upgrade/migration prompts
and sets an explicit path to the exact firmware package requested by the IOC.
The checked-in IOC and generated firmware sources are not edited or committed.
CubeMX is serialized per agent account to avoid simultaneous cache access.
Logs record the actual generation and compile commands; Jenkins records the Git
revision used for the pipeline definition as well as the checkout revision.

## Manual stage validation

Run from a fresh repository checkout under the intended build account:

```bash
export CUBEMX=/opt/st/STM32CubeMX/STM32CubeMX
export CUBEIDE_HOME=/opt/st/stm32cubeide
export FW_REPOSITORY=/opt/st/STM32Cube/Repository
export BUILD_CONFIG=Debug
bash ci/firmware.sh verify
bash ci/firmware.sh generate
bash ci/firmware.sh build
bash ci/firmware.sh package
```

Generation can also be tested independently while the current IDE is being
installed. A full successful build must be verified on that IDE; generation
alone is not a compilation test.

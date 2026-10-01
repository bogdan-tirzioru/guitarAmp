pipeline {
  agent any
  options {
    timestamps()
    disableConcurrentBuilds()
    skipDefaultCheckout(true)
    timeout(time: 30, unit: 'MINUTES')
    buildDiscarder(logRotator(numToKeepStr: '20', artifactNumToKeepStr: '10'))
  }
  parameters {
    string(name: 'GIT_REF', defaultValue: '*/main', description: 'Branch pattern or refs/tags/<tag>')
    choice(name: 'BUILD_CONFIG', choices: ['Debug', 'Release'], description: 'CubeIDE build configuration')
    string(name: 'CUBEMX', defaultValue: '/home/ghita/STM32CubeMX/STM32CubeMX', description: 'CubeMX executable accessible to the Jenkins agent')
    string(name: 'CUBEIDE_HOME', defaultValue: '/opt/st/stm32cubeide', description: 'Current H5E5-capable CubeIDE installation; 1.11 is unsupported')
    string(name: 'FW_REPOSITORY', defaultValue: '/home/ghita/STM32Cube/Repository', description: 'Preinstalled STM32Cube firmware packages accessible to the agent')
  }
  stages {
    stage('Checkout') {
      steps {
        deleteDir()
        checkout([$class: 'GitSCM',
          branches: [[name: params.GIT_REF]],
          extensions: [[$class: 'CloneOption', shallow: false, noTags: false, depth: 0]],
          userRemoteConfigs: [[
            url: 'https://github.com/bogdan-tirzioru/guitarAmp.git',
            credentialsId: 'github-bogdan-read',
            refspec: '+refs/heads/*:refs/remotes/origin/* +refs/tags/*:refs/tags/*'
          ]]
        ])
        sh 'mkdir -p ci-output && git rev-parse HEAD > ci-output/git-commit.txt'
      }
    }
    stage('Verify prerequisites') {
      steps { sh 'bash ci/firmware.sh verify' }
    }
    stage('Generate from IOC') {
      steps {
        timeout(time: 10, unit: 'MINUTES') {
          sh 'bash ci/firmware.sh generate'
        }
      }
    }
    stage('Compile and link') {
      steps {
        timeout(time: 15, unit: 'MINUTES') {
          sh 'bash ci/firmware.sh build'
        }
      }
    }
    stage('Package firmware') {
      steps {
        sh 'bash ci/firmware.sh package'
        archiveArtifacts artifacts: 'ci-output/firmware/*', fingerprint: true, allowEmptyArchive: false
      }
    }
  }
  post {
    always {
      archiveArtifacts artifacts: 'ci-output/*.log,ci-output/*.txt,ci-output/*.script,ci-output/*.ioc',
        allowEmptyArchive: true
    }
  }
}

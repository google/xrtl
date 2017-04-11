#!/usr/bin/env groovy

stage('Presubmit') {
  node('linux') {
    checkout scm
    sh('./tools/ci/jenkins/linux/presubmit.sh')
  }
}

stage('Build and Test') {
  def workers = [:]
  workers['linux'] = {
    node('linux') {
      checkout scm
      sh('./tools/ci/jenkins/linux/build_and_test.sh')
    }
  }
  workers['windows'] = {
    node('windows') {
      checkout scm
      bat('tools/ci/jenkins/windows/build_and_test.bat')
    }
  }
  workers.failFast = false
  parallel workers
}

stage('Analysis') {
  node('linux') {
    checkout scm
    sh('./tools/ci/jenkins/linux/analyze.sh')
    sh('./tools/ci/jenkins/linux/test_sanitize.sh asan')
    sh('./tools/ci/jenkins/linux/test_sanitize.sh msan')
    sh('./tools/ci/jenkins/linux/test_sanitize.sh tsan')
    sh('./tools/ci/jenkins/linux/test_sanitize.sh ubsan')
  }
}

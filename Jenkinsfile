#!/usr/bin/env groovy

stage('Presubmit') {
  node('linux') {
    checkout scm
    sh('./tools/ci/jenkins/linux/presubmit.sh')
  }
}

stage('Build and Test') {
  def workers = [:]
  def platforms = ['linux']
  for (platform in platforms) {
    workers[platform] = {
      node(platform) {
        checkout scm
        sh('./tools/ci/jenkins/' + platform + '/build_and_test.sh')
      }
    }
  }
  workers.failFast = false
  parallel workers
}

stage('Analysis') {
  node('linux') {
    checkout scm
    sh('./tools/ci/jenkins/linux/analyze.sh')
  }
}

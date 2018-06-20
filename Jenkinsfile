properties([parameters([
  booleanParam(defaultValue: true, description: 'Build `iroha`', name: 'iroha'),
  string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')])])

pipeline {
	environment {
		PREVIOUS_COMMIT = ''
		RESULT = ''
	}

	options {
    buildDiscarder(logRotator(numToKeepStr: '20'))
    timestamps()
  }

  agent any
  stages {
  	stage ('dry') {
  		agent { label 'master' }
  		steps {
  			script {
  				sh "[[ ! -z ${PREVIOUS_COMMIT} ]] && echo w"
  			}
  		}
  	}
  }
}
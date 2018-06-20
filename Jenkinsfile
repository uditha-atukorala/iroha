properties([parameters([
  booleanParam(defaultValue: true, description: 'Build `iroha`', name: 'iroha'),
  string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')])])

pipeline {
	environment {
		PREVIOUS_COMMIT = ''
		MERGE_CONDITIONS = ''
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
  			    def branch_ok = env.CHANGE_TARGET ==~ /(master|develop|trunk)/ ? "true" : "false"
  			    def pr_ok = env.CHANGE_ID && env.GIT_PREVIOUS_COMMIT ? "true" : "false"
  			    MERGE_CONDITIONS = (branch_ok == "true" && pr_ok == "true" && params.iroha) ? "true" : "false"
  			    println branch_ok
  			    println pr_ok
  			    println params.iroha
  			    println env.MERGE_CONDITIONS
  			}
  		}
  	}
  }
}
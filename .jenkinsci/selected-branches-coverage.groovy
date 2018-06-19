#!/usr/bin/env groovy

def selectedBranchesCoverage() {
	// trigger coverage if branch is either develop or master, or it is a PR
	def pCommit = load ".jenkinsci/previous-commit.groovy"
	def prevCommit = pCommit.previousCommitOrCurrent()
	def branch_coverage = ['master']

	if ( params.Coverage ) {
		return true
	}
	else {
		return env.GIT_LOCAL_BRANCH in branch_coverage || env.CHANGE_ID != null && env.GIT_COMMIT != prevCommit
	}
}

return this

#!/usr/bin/env groovy

def previousCommitOrCurrent() {
	// GIT_PREVIOUS_COMMIT is null on first PR build
	// regardless Jenkins docs saying it equals the current one on first build in branch
	return true
}

return this

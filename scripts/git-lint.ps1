##############################################################################
# Git Lint script
# (because there's no working git-linter for Windows right now)
#
# Usage:
#
# Validate current branch (diff from origin/develop):
# & ./git-lint.ps1
# 
# Validate commit message:
# & ./git-lint.ps1 "path/to/commmit-message"
#
# This script returns 1 on failure
##############################################################################

function Validate-Message($message) {
	$lines = $message -split [Environment]::NewLine
	$title = $lines[0]
	if ($title.length -gt 50) {
		throw [Exception] "First line should be at most 50 characters"
	} elseif ($title -Match '\.$') {
		throw [Exception] "First line should not end with a dot"
	} elseif ($title -NotMatch '^(ASSET|BREAK|FEAT|FIX|MAINT|REFACTOR|STYLE|TEST): ') {
		throw [Exception] "First line should start with ASSET:, BREAK:, FEAT:, FIX:, MAINT:, REFACTOR:, STYLE: or TEST:"
	} elseif ($lines.length -gt 1 -and $lines[1].length -gt 0) {
		throw [Exception] "Second line should be empty"
	} elseif (($lines | Where-Object {$_.length -gt 80}).length -gt 0) {
		throw [Exception] "Lines should have at most 80 characters"
	}
}

function Hashes {
	$hashes = git log origin/develop..HEAD --format="%h"
	return $hashes -Split [Environment]::NewLine
}

function Validate-Branch {
	$exitCode = 0;
	foreach ($hash in Hashes) {
		$body = git log -1 $hash --format="%B"
		try {
			Validate-Message($body)
		} catch [Exception] {
			Write-Host "$($hash) $($PSItem.ToString())"
			Write-Host $body
			$exitCode = 1
		}
	}
	return $exitCode
}

if ($args.length -gt 0) {
	$body = Get-Content -Path $args[0]
	try {
		Validate-Message($body)
	} catch [Exception] {
		Write-Host $PSItem.ToString()
		exit 1
	}
	exit 0
} else {
	exit Validate-Branch
}

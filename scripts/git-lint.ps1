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

$foundErrors = 0;
foreach ($hash in Hashes) {
	$body = git log -1 $hash --format="%B"
	try {
		Validate-Message($body)
	} catch [Exception] {
		Write-Host "$($hash) $($PSItem.ToString())"
		Write-Host $body
		$foundErrors = 1;
	}
}

exit $exitCode;

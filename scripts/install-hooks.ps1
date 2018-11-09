##############################################################################
#
# Install Git Hooks
#
##############################################################################

$oldLocation = Get-Location
cd $PSScriptRoot
cp git-hooks/* ../.git/hooks/
cd $oldLocation

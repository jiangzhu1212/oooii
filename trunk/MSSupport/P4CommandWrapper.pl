
$PerlEXE = "p4.exe";

sub SysCall
{
	$sysParams = $_[0];
	print "$sysParams\n   ";
	system($sysParams);
}

SysCall("$PerlEXE @ARGV");

$action = $ARGV[0];
$filePath = $ARGV[1];

if($filePath =~ /^.+\.vcxproj$/i)
{
	print("\n");
	SysCall("$PerlEXE $action $filePath.filters");
}

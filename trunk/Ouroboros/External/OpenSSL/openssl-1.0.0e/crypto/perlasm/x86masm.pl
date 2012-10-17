#!/usr/bin/env perl

package x86masm;

*out=\@::out;

$::lbdecor="\$L";	# local label decoration
$nmdecor="_";		# external name decoration

$initseg="";
$segment="";

sub ::generic
{ my ($opcode,@arg)=@_;

    # fix hexadecimal constants
    for (@arg) { s/0x([0-9a-f]+)/0$1h/oi; }

    if ($opcode !~ /movq/)
    {	# fix xmm references
	$arg[0] =~ s/\b[A-Z]+WORD\s+PTR/XMMWORD PTR/i if ($arg[1]=~/\bxmm[0-7]\b/i);
	$arg[1] =~ s/\b[A-Z]+WORD\s+PTR/XMMWORD PTR/i if ($arg[0]=~/\bxmm[0-7]\b/i);
    }

    &::emit($opcode,@arg);
  1;
}
#
# opcodes not covered by ::generic above, mostly inconsistent namings...
#
sub ::call	{ &::emit("call",(&::islabel($_[0]) or "$nmdecor$_[0]")); }
sub ::call_ptr	{ &::emit("call",@_);	}
sub ::jmp_ptr	{ &::emit("jmp",@_);	}

sub get_mem
{ my($size,$addr,$reg1,$reg2,$idx)=@_;
  my($post,$ret);

    $ret .= "$size PTR " if ($size ne "");

    $addr =~ s/^\s+//;
    # prepend global references with optional underscore
    $addr =~ s/^([^\+\-0-9][^\+\-]*)/&::islabel($1) or "$nmdecor$1"/ige;
    # put address arithmetic expression in parenthesis
    $addr="($addr)" if ($addr =~ /^.+[\-\+].+$/);

    if (($addr ne "") && ($addr ne 0))
    {	if ($addr !~ /^-/)	{ $ret .= "$addr";  }
	else			{ $post=$addr;      }
    }
    $ret .= "[";

    if ($reg2 ne "")
    {	$idx!=0 or $idx=1;
	$ret .= "$reg2*$idx";
	$ret .= "+$reg1" if ($reg1 ne "");
    }
    else
    {	$ret .= "$reg1";   }

    $ret .= "$post]";
    $ret =~ s/\+\]/]/; # in case $addr was the only argument
    $ret =~ s/\[\s*\]//;

  $ret;
}
sub ::BP	{ &get_mem("BYTE",@_);  }
sub ::DWP	{ &get_mem("DWORD",@_); }
sub ::QWP	{ &get_mem("QWORD",@_); }
sub ::BC	{ "@_";  }
sub ::DWC	{ "@_"; }

sub ::file
{ my $tmp=<<___;
TITLE	$_[0].asm
IF \@Version LT 800
ECHO MASM version 8.00 or later is strongly recommended.
ENDIF
.486
.MODEL	FLAT
OPTION	DOTNAME
IF \@Version LT 800
.text\$	SEGMENT PAGE 'CODE'
ELSE
.text\$	SEGMENT ALIGN(64) 'CODE'
ENDIF
___
    push(@out,$tmp);
    $segment = ".text\$";
}

sub ::function_begin_B
{ my $func=shift;
  my $global=($func !~ /^_/);
  my $begin="${::lbdecor}_${func}_begin";

    &::LABEL($func,$global?"$begin":"$nmdecor$func");
    $func="ALIGN\t16\n".$nmdecor.$func."\tPROC";

    if ($global)    { $func.=" PUBLIC\n${begin}::\n"; }
    else	    { $func.=" PRIVATE\n";            }
    push(@out,$func);
    $::stack=4;
}
sub ::function_end_B
{ my $func=shift;

    push(@out,"$nmdecor$func ENDP\n");
    $::stack=0;
    &::wipe_labels();
}

sub ::file_end
{ my $xmmheader=<<___;
.686
.XMM
IF \@Version LT 800
XMMWORD STRUCT 16
DQ	2 dup (?)
XMMWORD	ENDS
ENDIF
___
    if (grep {/\b[x]?mm[0-7]\b/i} @out) {
	grep {s/\.[3-7]86/$xmmheader/} @out;
    }

    push(@out,"$segment	ENDS\n");

    if (grep {/\b${nmdecor}OPENSSL_ia32cap_P\b/i} @out)
    {	my $comm=<<___;
.bss	SEGMENT 'BSS'
COMM	${nmdecor}OPENSSL_ia32cap_P:DWORD
.bss	ENDS
___
	# comment out OPENSSL_ia32cap_P declarations
	grep {s/(^EXTERN\s+${nmdecor}OPENSSL_ia32cap_P)/\;$1/} @out;
	push (@out,$comm);
    }
    push (@out,$initseg) if ($initseg);
    push (@out,"END\n");
}

sub ::comment {   foreach (@_) { push(@out,"\t; $_\n"); }   }

*::set_label_B = sub
{ my $l=shift; push(@out,$l.($l=~/^\Q${::lbdecor}\E[0-9]{3}/?":\n":"::\n")); };

sub ::external_label
{   foreach(@_)
    {	push(@out, "EXTERN\t".&::LABEL($_,$nmdecor.$_).":NEAR\n");   }
}

sub ::public_label
{   push(@out,"PUBLIC\t".&::LABEL($_[0],$nmdecor.$_[0])."\n");   }

sub ::data_byte
{   push(@out,("DB\t").join(',',@_)."\n");	}

sub ::data_word
{   push(@out,("DD\t").join(',',@_)."\n");	}

sub ::align
{   push(@out,"ALIGN\t$_[0]\n");	}

sub ::picmeup
{ my($dst,$sym)=@_;
    &::lea($dst,&::DWP($sym));
}

sub ::initseg
{ my $f=$nmdecor.shift;

    $initseg.=<<___;
.CRT\$XCU	SEGMENT DWORD PUBLIC 'DATA'
EXTERN	$f:NEAR
DD	$f
.CRT\$XCU	ENDS
___
}

sub ::dataseg
{   push(@out,"$segment\tENDS\n_DATA\tSEGMENT\n"); $segment="_DATA";   }

1;

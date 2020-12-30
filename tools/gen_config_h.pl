#!/usr/bin/perl -w
use strict;
use warnings;

my $debug = 0;

my ($target) = @ARGV;

print(STDERR "Generating src/config.h\n");

my $h = "// Automatically generated file. Do not edit.\n\n"
      . "#ifndef __CONFIG_H__\n"
      . "#define __CONFIG_H__\n\n"
      . "#define CONFIG_VERSION_GIT_HASH \"" . getGithash() . "\"\n"
      . "#define CONFIG_VERSION_YYYYMMDD \"" . getDate() . "\"\n"
      . "#define CONFIG_VERSION_HHMMSS   \"" . getTime() . "\"\n"
      . "\n" . slurp("src/config-common.txt")
      . "\n" . slurp("src/config-$target.txt")
      . "\n\n#endif\n";

print(STDERR $h) if ($debug);

my $configH = 'src/config.h';
open(OUT, '>', $configH) || die("Cannot write $configH: $!");
print(OUT $h);
close(OUT);

sub getDate
{
    my (undef, undef, undef, $mday, $mon, $year) = localtime();
    return sprintf('%04i-%02i-%02i', $year + 1900, $mon + 1, $mday);
}

sub getTime
{
    my ($sec, $min, $hour) = localtime();
    return sprintf('%02i:%02i:%02i', $hour, $min, $sec);
}

sub getGithash
{
    my ($hash, $dirty);
    if ($^O =~ m/Win/i)
    {
        $hash = qx{git log --pretty=format:%H -n1};
        $dirty = qx{git describe --always --dirty};
    }
    else
    {
        $hash = qx{LC_ALL=C git log --pretty=format:%H -n1 2>/dev/null};
        $dirty = qx{LC_ALL=C git describe --always --dirty 2>/dev/null};
    }
    $hash =~ s{\r?\n}{};
    $dirty =~ s{\r?\n}{};
    print(STDERR "hash=[$hash]\ndirty=[$dirty]\n") if ($debug);

    if ($hash && $dirty)
    {
        my $str = substr($hash, 0, 8);
        if ($dirty =~ m{-dirty$})
        {
            $str .= 'M' ;
        }
        return $str;
    }
    else
    {
        return "00000000";
    }
}

sub slurp
{
    my ($file) = @_;
    local $/;
    open(F, '<', $file) || die("Cannot read $file: $!");
    my $c = <F>;
    close(F);
    return "// $file\n$c";
}

# eof

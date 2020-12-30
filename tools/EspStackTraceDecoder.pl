#!/usr/bin/perl -w
#
# ESP8266 Exception Decoder
#
# Copyright (c) 2020 Philippe Kehl <flipflip at oinkzwurgl dot org>
#
# Perl version of https://github.com/me-no-dev/EspExceptionDecoder
#
# Copyright (c) 2015 Hristo Gochkov (ficeto at ficeto dot com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
use strict;
use warnings;

my @exceptions =
(
    "Illegal instruction",
    "SYSCALL instruction",
    "InstructionFetchError: Processor internal physical address or data error during instruction fetch",
    "LoadStoreError: Processor internal physical address or data error during load or store",
    "Level1Interrupt: Level-1 interrupt as indicated by set level-1 bits in the INTERRUPT register",
    "Alloca: MOVSP instruction, if caller's registers are not in the register file",
    "IntegerDivideByZero: QUOS, QUOU, REMS, or REMU divisor operand is zero",
    "reserved",
    "Privileged: Attempt to execute a privileged operation when CRING ? 0",
    "LoadStoreAlignmentCause: Load or store to an unaligned address",
    "reserved",
    "reserved",
    "InstrPIFDataError: PIF data error during instruction fetch",
    "LoadStorePIFDataError: Synchronous PIF data error during LoadStore access",
    "InstrPIFAddrError: PIF address error during instruction fetch",
    "LoadStorePIFAddrError: Synchronous PIF address error during LoadStore access",
    "InstTLBMiss: Error during Instruction TLB refill",
    "InstTLBMultiHit: Multiple instruction TLB entries matched",
    "InstFetchPrivilege: An instruction fetch referenced a virtual address at a ring level less than CRING",
    "reserved",
    "InstFetchProhibited: An instruction fetch referenced a page mapped with an attribute that does not permit instruction fetch",
    "reserved",
    "reserved",
    "reserved",
    "LoadStoreTLBMiss: Error during TLB refill for a load or store",
    "LoadStoreTLBMultiHit: Multiple TLB entries matched for a load or store",
    "LoadStorePrivilege: A load or store referenced a virtual address at a ring level less than CRING",
    "reserved",
    "LoadProhibited: A load referenced a page mapped with an attribute that does not permit loads",
    "StoreProhibited: A store referenced a page mapped with an attribute that does not permit stores"
);

my $addr2line = 'addr2line';

my ($elfFile, $dumpFile) = @ARGV;
if (!$addr2line || !$elfFile || !$dumpFile)
{
    printf("Usage: $0 /path/to/program.elf /path/to/dump/of/exception.txt\n");
    exit(1);
}

my $dumpData = '';
if ($dumpFile eq '-')
{
    print("Paste stack trace here and press CTRL-D\n");
    local $/;
    $dumpData = <STDIN>;
}
else
{
    open(DUMP, '<', $dumpFile) || die("Cannot open $dumpFile: $!");
    $dumpData = do { local $/; <DUMP> }; # slurp
    close(DUMP);
}

# extract exception
my $exception = ($dumpData =~ m{Exception ([0-9]+):}s) ? $1 : undef;
print((defined $exception && $exceptions[$exception] ? $exceptions[$exception]
    : 'Unknown exception' . ($exception // '')) . "\n");

# decode stack trace (consider anything that looks like a function address)
my @addrs = ($dumpData =~ m{(40[0-2][0-9a-f]{5})\b}mg);
if ($#addrs > -1)
{
    print qx{$addr2line -aipfC -e $elfFile @addrs};
}
else
{
    print("No useful addresses found.\n");
}

# eof

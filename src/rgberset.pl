#!/usr/bin/perl
use strict;
use warnings;
use GD;
my $img = GD::Image->new('rgberset.png');
my ($w, $h) = $img->getBounds();
printf("static const uint8_t rgberset[$w*$h] =\n{\n   ");
for (my $x = 0; $x < $w; $x++)
{
    for (my $y = 0; $y < $h; $y++)
    {
        my $ix = $img->getPixel($x, $y);
        my ($r, $g, $b) = $img->rgb($ix);
        printf(" 0x%02x,", ($r + $g + $b) / 3);
    }
    print("\n   ");
}
print("\n};\n");
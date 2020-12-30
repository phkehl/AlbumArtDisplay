####################################################################################################

package Ffi::Debug;

=pod

=encoding utf-8

=head1 Ffi::Debug -- debugging stuff

Common debugging and message output related stuff.

=head2 Description

This module provides several functions to output messages. The default output
device is a colourful C<STDERR>, but may be changed by replacing the output
function (see below). The module has several verbosity levels that can be
adjusted if required.

The C<__WARN__> and C<__DIE__> signals (see L<perlvar>) are linked to the
C<WARN> and C<DIE> functions, which automatically add a convenient stack trace
incl. function/method arguments to the message. C<__WARN__> and C<__DIE__>
signals are raised by Perl on warnings (such as the use of an unitialised
variable) or sever problems (such as calling a nonexistent method on an
object), respectively. You can raise those signals manually, too (see examples
below). The C<__DIE__> handler with try to figure out if the signal has been
raised in an C<eval()> or not. If it did it will C<PANIC()> the error message
and not exit the program (which it will do otherwise).

Note that for CGI scripts the C<__WARN__> and C<__DIE__> it will do special handling, too. It will
try hard to produce some useful debug and error output to the web browser in case of syntax errors
etc. when compiling the script. It's similar to what L<CGI::Carp> does with its I<fatalsToBrowser>
option. You should not mix this module with L<CGI::Carp>.

=head2 Examples

    use Ffi::Debug ':all';

    NOTICE('Good morning, good evening!');
    DEBUG('The program has now started.');

    if ($somethingWentWrong)
    {
        ERROR("Something went wrong!");
    }

    open(F, '>', 'blabla.dat') || die("Ouch, cannot open blabla.dat for writing: $!");

    TRACE("some data: %s", \%someHash);

    SUCCESS('All done!');

This won't print any of the DEBUG() or TRACE() messages. If you want to see those
messages you have two options:


=over

=item * increase the verbosity level in the script

   $Ffi::Debug::VERBOSITY += 2;  # enables DEBUG() and TRACE()

=item * set the environment variable FFI_DEBUG when running the script:

   $ FFI_DEBUG=2 ./myscript.pl

=back

The module will enable stack traces for C<__WARN__> and C<__DIE__> signals as
well as for the C<PANIC()> call. Consider the following script:

    #!/usr/bin/perl
    
    use warnings;
    use strict;
    
    use Ffi::Debug ':all';
    $Ffi::Debug::TIMESTAMP = 2;
    
    meier(1, { some => 1, args => 2 }, undef);
    
    sub meier
    {
        warn('Ouch!');
        PRINT("still running..");
        PANIC('Sackzemänt!');
        PRINT("still running..");
        DIE('Dammisiech!');
        PRINT("died..");
    }

This will output (of course, nicely colourd):

    00000.000 Ouch! at test.pl line 13.
              frame 0: at test.pl line 13
              frame 1: main::meier(1, 'HASH(0x1ff0c3d0)', undef) called at test.pl line 9
    00000.002 still running..
    00000.002 Sackzem\x{c3}\x{a4}nt!
              frame 0: at test.pl line 15
              frame 1: main::meier(1, 'HASH(0x1ff0c3d0)', undef) called at test.pl line 9
    00000.002 still running..
    00000.002 Dammisiech!
              frame 0: at test.pl line 17
              frame 1: main::meier(1, 'HASH(0x1ff0c3d0)', undef) called at test.pl line 9

Or with additional tracing (FFI_STACK=1, see below):

    00000.000 Ouch! at test.pl line 13.
              frame 0: at test.pl line 13
              frame 1: main::meier(1, 'HASH(0xe7b03d0)', undef) called at test.pl line 9
    00000.001 still running..
              at test.pl line 14
    00000.002 Sackzem\x{c3}\x{a4}nt!
              frame 0: at test.pl line 15
              frame 1: main::meier(1, 'HASH(0xe7b03d0)', undef) called at test.pl line 9
    00000.002 still running..
              at test.pl line 16
    00000.002 Dammisiech!
              frame 0: at test.pl line 17
              frame 1: main::meier(1, 'HASH(0xe7b03d0)', undef) called at test.pl line 9

The C<PANIC()> or C<TRACE()> stack frames come in handy for debugging. Consider this example:

    $SIG{USR1} = sub { PANIC('Caught SIGUSR1!'); };

    ...lot's of code, loops and stuff...

You can now send a the USR1 signal to the process and it will print 'Caught SIGUSR1' along with a
full stack trace. This is very handy to debug programs, e.g. one that is hanging in some loop or
taking too much time. Repeatedly send the USR1 signal and analyse the stack traces and you should be
able to tell where it is hanging or spending most time.

=cut

use strict;
use warnings;

use File::stat; # this magically fixes some strange error on newer Perls..
use Time::HiRes qw(time);
use POSIX;
use FindBin;
#use Carp;
#$Carp::CarpLevel = 2;
#our @CARP_NOT;

# exported interface
use base 'Exporter';
our %EXPORT_TAGS = ( all => [ qw(PANIC DIE
                                 SUCCESS NOTICE FAILURE WARNING ERROR
                                 PRINT
                                 DEBUG
                                 TRACE) ] );
our @EXPORT_OK   = ( @{ $EXPORT_TAGS{'all'} } );


###############################################################################

=pod

=head2 Runtime Configuration

The following module variables may be changed (FIXME: we should probably provide a decent interface
to these). Note that you may only use that in scripts (C<.pl>) and that you must not use it in
modules (C<.pm>).

=over

=item C<$VERBOSITY>

The verbosity levels (ordered by increasing verbosity) are:

=over

=item * PANIC, DIE

=item * SUCESS, NOTICE, WARNING, ERROR, FAILURE

=item * PRINT -- the default level

=item * DEBUG

=item * TRACE

=back

To change the verbosity levels you either I<increase> or I<decrease> the
C<$VERBOSITY> value. E.g. to make a script rather quiet and output only severe
stuff (C<PANIC()> and C<DIE()>) decrease the verbosity by 2: 

    $Ffi::Debug::VERBOSITY -= 2;

To enable all debugging messages increase the verbosity by 3:

    $Ffi::Debug::VERBOSITY += 3;

Alternatively you can set the environment variable C<FFI_DEBUG> to 2 for the
same effect. See also the example above.

=item C<$TIMESTAMP>

Prefix timestamps to all messages (0 = no, 1 = yes, 2 = relative, 3 = absolute).
Default: 0 or what the C<FFI_TIMESTAMP> environment variable says.

=item C<$PRINTPID>

Print PID with all messages (0 = no, 1 = yes). Default: 0.

=item C<$PRINTNAME>

Print name with all messages ('' = no, 'something' = print 'something'). Default: ''.

=item C<$PRINTTYPE>

Add a message type char to the output (0 = no, 1 = yes). Default: 0.

The type chars are (see also C<%msgTypeIds>): B<X> (PANIC), B<%> (DIE), B<!>
(WARN), B<W> (WARNING), B<S> (SUCCESS), B<F> (FAILURE), B<E> (ERROR), B<N>
(NOTICE), B<P> (PRINT), B<1> (DEBUG), B<2> (TRACE)


=item C<$STACK>

Add function adds a stack frame dump to the messages (0 = no, 1 = yes, 2 = full stack). Can be
enabled by setting the C<FFI_STACK> environment variable to 1 (or 2).

=item C<$RENDERFUNC>

Points to the function that renders the message into a printable version (a
string). See the C<_RENDER()> function below.

=item C<$PRINTFUNC>

Points to the function that prints the rendered message. See the C<_OUTPUT()> function below.

=item C<$PRINTFH>

The file handle used by the default C<$PRINTFUNC>.

=item C<$COLOURS>

Whether colours shall be used or not (0 = no, 1 = yes). The default is
auto-detect (i.e. 'yes' if C<STDERR> is on an interactive terminal). Can be
forced to either side by setting the C<FFI_COLOURS> environment variable
accordingly.

=item C<$MULTILINE>

Whether to render complicated data structures on multiple lines or not.

=item C<$PREFIX>

Regex to apply to stack traces (to remove a prefix from paths). The module's
C<INIT()> function automatically tries to set a reasonable default.

=item C<$MARKER>

A string that is prepended to all messages. Empty by default.

=item C<$FLATTEN>

Try to flatten structure dumps into something more readable. Experimental. YMMV.

=back

=cut

our $VERBOSITY  = 3;
our $TIMESTAMP  = 0;          # 0 = no, 1 = yes, 2 = relative
our $PRINTPID   = 0;          # 0 = no, 1 = yes
our $PRINTNAME  = '';         # '' = no, '...' = that...
our $PRINTTYPE  = 0;          # 0 = no, 1 = yes
our $STACK      = 0;          # 0 = no, 1 = yes, 2 = full
our $COLOURS    = 1;          # 0 = no, 1 = yes (default set below in INIT())
our $MULTILINE  = 1;          # 0 = no, 1 = yes
our $PREFIX     = undef;      # a regex, automatically set by INIT()
our $FORMATFUNC = \&_FORMAT;  # the function that formats the user message
our $RENDERFUNC = \&_RENDER;  # the function that renders the message into a string
our $PRINTFUNC  = \&_OUTPUT;  # the function that does the actual output
our $PRINTFH    = *STDERR;    # a file handle
our $MARKER     = '';         # marker prepended to all messages
our $FLATTEN    = 1;          # 0 = no, 1 = yes

###############################################################################
# module initialisation

our $T0;

our $VERSION = '2.0';

BEGIN
{
    # in Windoze, check if we have the appropriate colours translater
    if ( $^O =~ m/Win32/io )
	{
		eval "local \$^W = 0; require Win32::Console::ANSI;";
		if ($@) # fails loading properly on newer Windozen
		{
			undef $Win32::Console::ANSI::VERSION;
		}
	}
}

INIT
{
    # enable colours by default if we're printing to an interactive terminal
    $COLOURS = (-t $PRINTFH ? 1 : 0);

    # override defaults based on environment variables
    if (defined $ENV{FFI_DEBUG} && ($ENV{FFI_DEBUG} > -1))
    {
        $VERBOSITY = int( 3 + $ENV{FFI_DEBUG} );
    }
    if (defined $ENV{FFI_STACK} && ($ENV{FFI_STACK} > -1))
    {
        $STACK = int( $ENV{FFI_STACK} );
    }
    if ((defined $ENV{FFI_COLOURS} && ($ENV{FFI_COLOURS} > -1)) ||
        (defined $ENV{FFI_COLORS}  && ($ENV{FFI_COLORS} > -1)))
    {
        $COLOURS = int( defined $ENV{FFI_COLOURS} ? $ENV{FFI_COLOURS} : $ENV{FFI_COLORS} );
    }

    if ( $COLOURS && ($^O =~ m/Win32/i) && !defined $Win32::Console::ANSI::VERSION )
    {
		$COLOURS = 0;
        DEBUG("Could not load Win32::Console::ANSI! Not using colours.. :-(");
    }

    if (defined $ENV{FFI_TIMESTAMP} && ($ENV{FFI_TIMESTAMP} > -1))
    {
        $TIMESTAMP = $ENV{FFI_TIMESTAMP};
    }

    $PREFIX = qr{$FindBin::Bin/?};

    $T0 = time();
}

###############################################################################
# our colours (optimised for a dark background)
our %C = (
          PANIC => "\e[1;31m", DIE => "\e[1;31m", DIENT => "\e[1;31m",
          SUCCESS => "\e[32m", NOTICE => "\e[1m", WARNING => "\e[33m", ERROR => "\e[31m", FAILURE => "\e[31m",
          PRINT => "\e[m",
          DEBUG => "\e[36m", TRACE => "\e[36m",

          WARN => "\e[33m",
          STACK => "\e[35m", TIMESTAMP => "\e[34m", PID => "\e[35m", OFF => "\e[m",
         );

# and the translation to HTML (optimised for a bright background)
our %C2H = (
            "\e[1;31m" => [ '<span style="color: #f00; font-weight: bold;">', '</span>' ], # PANIC, DIE
            "\e[32m"   => [ '<span style="color: #0a0;">', '</span>' ], # SUCCESS
            "\e[1m"    => [ '<span style="font-weight: bold;">', '</span>' ], # NOTICE
            "\e[33m"   => [ '<span style="color: #aa0;">', '</span>' ], # WARNING, WARN
            "\e[31m"   => [ '<span style="color: #f00;">', '</span>' ], # ERROR, FAILURE
            "\e[m"     => [ '', '' ],  # PRINT
            "\e[36m"   => [ '<span style="color: #aaa;">', '</span>' ], # DEBUG, TRACE
            "\e[35m"   => [ '<span style="color: #505;">', '</span>' ], # STACK
            "\e[34m"   => [ '<span style="color: #555;">', '</span>' ], # TIMESTAMP
            "\e[35m"   => [ '<span style="color: #50f;">', '</span>' ], # PID
           );


###############################################################################

=pod

=head2 API

All functions are designed to take any kind of argument (scalars, hash
references, etc.) while providing a human readable output. If the first
argument looks like a format string and there is at least one more argument
C<sprintf()> is used to format the resulting messages. Arguments that are not
scalars (e.g.. hash and array references) are nicely formatted to strings (see
examples above). Each argument goes onto one output line.

=over

=item C<< PANIC() >>

prints a panic message

=item C<< DIE() >>

prints a death message and then exits the progam, same effect as die().

=item C<< SUCCESS() FAILURE() >>

prints success and failure messages (think failed/passed tests)

=item C<< WARNING() >>

prints a warning message (not to be confused by the Perl internal function C<warn()>)

=item C<< ERROR() >>

prints an error message

=item C<< NOTICE() >>

prints a notice, i.e. something worth mentioning but of no particular importance (unlike e.g. C<ERROR()>)

=item C<< PRINT() >>

prints all the stuff nobody ever reads

=item C<< DEBUG() >>

debugging messages

=item C<< TRACE() >>

tracing messages

=item C<< $str = Ffi::Debug::xsprintf(...) >>

Returns the formatted string as C<PRINT()> et al. would print it, minus all the decorations
(colours, timestamps, etc.).

=cut

sub PANIC
{
    return unless ($VERBOSITY > 0);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'PANIC', @_ ) ) );
}

sub DIE
{
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'DIE', @_ ) ) );
    exit(1);
}

sub SUCCESS
{
    return unless ($VERBOSITY > 1);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'SUCCESS', @_ ) ) );
}

sub FAILURE
{
    return unless ($VERBOSITY > 1);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'FAILURE', @_ ) ) );
}

sub NOTICE
{
    return unless ($VERBOSITY > 1);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'NOTICE', @_ ) ) );
}

sub WARNING
{
    return unless ($VERBOSITY > 1);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'WARNING', @_ ) ) );
}

sub ERROR
{
    return unless ($VERBOSITY > 1);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'ERROR', @_ ) ) );
}

sub PRINT
{
    return unless ($VERBOSITY > 2);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'PRINT', @_ ) ) );
}

sub DEBUG
{
    return unless ($VERBOSITY > 3);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'DEBUG', @_ ) ) );
}

sub TRACE
{
    return unless ($VERBOSITY > 4);
    $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'TRACE', @_ ) ) );
}

$SIG{__WARN__} = sub { return unless ($VERBOSITY > 1);
                       my $msg = $_[0];
                       $msg = "Warning: $msg" unless ($msg =~ m{warning}i);
                       $msg =~ s/\n$//;
                       $msg =~ s/$PREFIX//g if ($PREFIX);
                       $PRINTFUNC->( $RENDERFUNC->( $FORMATFUNC->( 'WARN', $msg) ) ); };

$SIG{__DIE__}  = sub { my $msg = $_[0];
                       $msg = "Error: $msg" unless ($msg =~ m{error}i);
                       $msg =~ s/\n$//;
                       $msg =~ s/$PREFIX//g if ($PREFIX);

                       # parsing --> complain and abort
                       if (!defined $^S)
                       {
                           my @out = $RENDERFUNC->( $FORMATFUNC->( 'DIENT', # DIE() colours w/o stack dump
                               "", "***** Parsing/compilation problem(s) in ${^GLOBAL_PHASE} *****", "", map { "- $_" } split(/\n/, $msg)));
                           if ($ENV{GATEWAY_INTERFACE}) # CGI scripts
                           {
                               print(renderHtmlError(@out));
                           }
                           else
                           {
                               $PRINTFUNC->(@out);
                           }
                           exit(1);
                       }
                       # in eval --> just complain with full stack dump
                       elsif ($^S)
                       {
                           PANIC(split(/\n/, $msg));
                       }
                       # not in eval --> complain with full stack dump and abort
                       else
                       {
                           if ($ENV{GATEWAY_INTERFACE}) # CGI scripts
                           {
                               my ($out, undef) = $RENDERFUNC->( $FORMATFUNC->('DIE', split(/\n/, $msg)));
                               print(renderHtmlError($out));
                           }
                           else
                           {
                               DIE(split(/\n/, $msg));
                           }
                       }
                   };

sub xsprintf
{
    return $FORMATFUNC->('DEBUG', @_)->{text};
}

sub renderHtmlError
{
    return ("Content-type: text/html\n\n",
            '<html><head>',
            '<link href="data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADwPwAA4B8AAOAPAADgAwAA4BkAAOAdAADwPQAA+DkAAPh7AADwMwAA4BcAAOATAADwOwAA8D8AAPM/AAD3vwAA" rel="icon" type="image/x-icon" />',
            '</head><body>',
            '<h1>666 Ouch!</h1>',
            '<pre>', toHtml(@_), '</pre></body></html>');
}

###############################################################################

=pod

=item C<< $htmlCode = Ffi::Debug::toHtml($lineOfText, ...) >>

Transforms ANSI colour codes and HTML entities to HTML.

=cut

sub toHtml
{
    my $html = '';
    foreach my $line ( @_ )
    {
        $line =~ s/</&lt;/g;
        $line =~ s/>/&gt;/g;

        my $out = '';
        my $markup = undef;
        foreach my $part (split(/(\e\[[^m]*m)/, $line))
        {
            next if ($part eq '');
            if ($part =~ m/^\e/ && defined $C2H{$part})
            {
                $markup = $C2H{$part};
            }
            else
            {
                $out .= $markup->[0] if ($markup);
                $out .= $part;
                $out .= $markup->[1] if ($markup);
                $markup = undef;
            }
        }

        #$out =~ s/[^m]*m//sg;      # remove ansi escape sequences

        $html .= $out;
    }
    return $html;
}



=pod

=back

=cut

###############################################################################
# INTERNAL FUNCTIONS

# $msg = _FORMAT(type, mixed)
# Takes a type (like 'ERROR', 'DEBUG', etc.) and mixed arguments (see the
# interface description above) and returns a message hash reference with the
# following fields: I<type> -- the type of the message ('NOTICE', 'DEBUG', etc.);
# I<time> -- a timestamp ([s] since epoch); I<pid> -- the PID; I<text> -- the
# formatted text; and optionally I<stack> -- the stack dump.
sub _FORMAT
{
    my $msg = {};
    my @args = @_;

    ###### meta data
    my $type = shift(@args);
    $msg->{type} = $type;
    $msg->{time} = time();
    $msg->{pid} = $$;

    ###### make text
    if ($#_ > -1)
    {
        # check for a format string and arguments if the first argument is a scalar
        #print("--> 0 = [$_[0]] #=" . ($#_+1) ." [" . ref($_[0]) . "]\n");
        if ( (defined $args[0]) && (ref($args[0]) eq '') && ($args[0] =~ m/%/) && ($#args > 0) && ($type ne 'DIE') )
        {
            # escape and format all arguments
            #print("--> sprintf [" . join("] [", @args) ."]!\n");
            my $fmt = shift(@args);
            #print("fmt=[$fmt]\n");
            my @sprintfArgs = ();
            push(@sprintfArgs, _formatValue($_)) for (@args);
            $msg->{text} = sprintf($fmt, @sprintfArgs);
        }
        else
        {
            if ($#args == 0) # one arg
            {
                $msg->{text} = _formatValue(@args);
            }
            else # many args -> treat as array reference
            {
                for (my $ix = 0; $ix <= $#args; $ix++)
                {
                    $msg->{text} .= _formatValue($args[$ix]) . ($ix < $#args ? "\n" : '');
                }
            }
        }
    }
    else
    {
        $msg->{text} = '';
    }

    # prepend marker
    substr($msg->{text}, 0, 0, $MARKER) if ($MARKER);

    ##### add stack dump
    if ( ($type eq 'PANIC') || ($type eq 'DIE') || ($type eq 'WARN') )
    {
        $msg->{stack} = _stack(2);
    }
    elsif ($type eq 'TRACE')
    {
        $msg->{stack} = _stack($STACK > 0 ? 2 : 1);
    }
    elsif ($STACK)
    {
        $msg->{stack} = _stack($STACK > 1 ? 2 : 1);
    }
    #else
    #{
    #    $msg->{stack} = '';
    #}


    return $msg;
}


our %msgTypeIds = ( PANIC => 'X', DIE => '%', WARN => '!', WARNING => 'W',
                    SUCCESS => 'S', FAILURE => 'F', ERROR => 'E',
                    NOTICE => 'N', PRINT => 'P', DEBUG => 'D', TRACE => 'T' );
# ($string, $typeId) = _RENDERS( \%msg ) >>
# Renders a message formatted by C<_FORMAT()> into a printable string.
sub _RENDER
{
    my $msg = shift;
    my $type = $msg->{type};

    my $out = '';
    my $indent = 0;

    # timestamp?
    if ($TIMESTAMP > 2)
    {
        $out .= $C{TIMESTAMP} if ($COLOURS);
        $out .= strftime('%Y-%m-%d %H:%M:%S', gmtime($msg->{time}));
        $out .= sprintf('.%03i', ($msg->{time} - int($msg->{time})) * 1e3);
        $out .= $C{OFF} if ($COLOURS);
        $out .= ' ';
        $indent += 24;
    }
    elsif ($TIMESTAMP > 1)
    {
        $out .= $C{TIMESTAMP} if ($COLOURS);
        $out .= sprintf("%09.3f", $msg->{time}-$T0);
        $out .= $C{OFF} if ($COLOURS);
        $out .= ' ';
        $indent += 10;
    }
    elsif ($TIMESTAMP > 0)
    {
        $out .= $C{TIMESTAMP} if ($COLOURS);
        $out .= strftime('%Y-%m-%d %H:%M:%S', gmtime($msg->{time}));
        $out .= $C{OFF} if ($COLOURS);
        $out .= ' ';
        $indent += 20;
    }

    # message type?
    if ($PRINTTYPE)
    {
        $out .= $C{TIMESTAMP} if ($COLOURS);
        $out .= $msgTypeIds{$type};
        $out .= $C{OFF} if ($COLOURS);
        $out .= ' ';
        $indent += 2;
    }

    # PID and/or name?
    if ($PRINTPID || $PRINTNAME)
    {
        $out .= $C{PID} if ($COLOURS);
        my $pid = $PRINTPID && $PRINTNAME ? "$msg->{pid}/$PRINTNAME" : $PRINTPID ? "$msg->{pid}" : $PRINTNAME;
        $out .= $pid;
        $out .= $C{OFF} if ($COLOURS);
        $out .= ' ';
        $indent += length($pid) + 1;
    }

    # the message
    $indent = ' ' x $indent;
    my $text = $msg->{text};
    if ($COLOURS) # colour each output line individually
    {
        $text =~ s/^/$C{$type}/mgo;
        $text =~ s/$/$C{OFF}/mgo;
    }
    $text =~ s/\n/\n$indent/g; # if ($MULTILINE);
    $out .= $text;

    if ($msg->{stack})
    {
        foreach (@{$msg->{stack}})
        {
            # colour and indent each output line individually
            $out .= "\n";
            $out .= $indent . '';
            $out .= $C{STACK} if ($COLOURS);
            $out .= $_;
            $out .= $C{OFF}   if ($COLOURS);
        }
    }

    return $out, $msgTypeIds{$type};
}


# _OUTPUT( $string )
# Takes a string (output of C<_RENDER()> and prints it to C<$PRINTFH>.
sub _OUTPUT
{
    print $PRINTFH $_[0] . "\n";
}


# $string = _formatValue(mixed
# Formats stuff nicely.
our %seen;
sub _formatValue
{
    my $v = shift; $v = 'undef' unless (defined $v);
    my $multiLine = shift || undef; $multiLine = $MULTILINE if (!defined $multiLine);
    my $nestLevel = shift || 0;
    my $v2;
    my $r = '';

    if ($nestLevel == 0)
    {
        %seen = ();
    }

    my $indent = ($multiLine ? (' ' x ($nestLevel * 4)) : '');

    # not using ref() because that might return the bless()ed name
    if (UNIVERSAL::isa($v, 'ARRAY'))
    {
        if ($seen{"$v"})
        {
            $r .= '[ ' . $seen{"$v"} . ' ]';
            return $r;
        }
        $seen{"$v"} = "$v";

        $r .= ($#{$v} > -1 ? ($multiLine && $nestLevel ? "\n" : '') . $indent : '' ) . '[ ';
        $indent = ($multiLine ? "\n" . (' ' x (($nestLevel+1)*4)) : '');
        for (my $ix = 0; $ix <= $#{$v}; $ix++)
        {
            my $v2 = $v->[$ix];
            $r .= $indent . _formatValue($v2, $multiLine, $nestLevel+1) . ($ix < $#{$v} ? ', ' : ' ');
        }
        $indent = ($multiLine && ($#{$v} > -1) ? "\n" . (' ' x (($nestLevel)*4)) : '');
        $r .= $indent . ']';
    }
    elsif (UNIVERSAL::isa($v, 'HASH'))
    {
        if ($seen{"$v"})
        {
            $r .= '{ '. $seen{"$v"} . ' }';
            return $r;
        }
        $seen{"$v"} = "$v";

        my @keys = sort keys %{$v};
        $r .= ($#keys > -1 ? ($multiLine && $nestLevel ? "\n" : '') . $indent : '' ) 
          . '{' . (ref($v) ne 'HASH' ? ' # ' . ref($v) : '');

        $indent = ($multiLine ? "\n" . (' ' x (($nestLevel+1)*4)) : '');
        for (my $ix = 0; $ix <= $#keys; $ix++)
        {
            my $v2 = $keys[$ix];
            $r .= "$indent$v2 => " . _formatValue($v->{$v2}, $multiLine, $nestLevel+1) . ($ix < $#keys ? ', ' : ' ');
        }
        $indent = ($multiLine && ($#keys > -1) ? "\n" . (' ' x (($nestLevel)*4)) : '');
        $r .= $indent . '}';
    }
    else
    {
        # value contains unprintable stuff -> binary dump
        if ($v =~ m/[^[:print:]]+/)
        {
            # maybe we only have some \r, \n and/or \t
            my $t = $v;
            $t =~ s/\r/\\r/g;
            $t =~ s/\n/\\n/g;
            $t =~ s/\t/\\t/g;
            # now printable?
            if ($t !~ m/[^[:print:]]+/)
            {
                # quote?
                if ($nestLevel && ($t !~ m/^-?[0-9\.]+$/))
                {
                    $t =~ s/"/\"/g;
                    $r = '"' . $t . '"';
                }
                else
                {
                    $r = $t;
                }
            }
            # initial PRINT() argument was not a simple string
            elsif ($nestLevel)
            {
                $r .= '[ ';
                # dump words in network order (big-endian)
                $r .= join(', ',
                           map { sprintf("0x%08x", $_); }           # format words
                           unpack('N*',                             # decode as many complete words as there are
                                  $v . pack('C3', 0x00, 0x00, 0x00) # pad with null values
                                 )
                          );
                $r .= ' ]';
            }
            # a simple string
            else
            {
                #$v =~ s/([[:cntrl:]]|[[:^asci:]])/sprintf("\\x{%x}",ord($1))/eg;
                $v =~ s/([[:cntrl:]])/sprintf("\\x{%x}",ord($1))/eg;
                $r .= $v;
            }

        }
        else
        {
            # use as-is
            if ($nestLevel && ($v ne 'undef') && ($v !~ m/^-?[0-9\.]+$/)) # quote?
            {
                $v =~ s/"/\"/g;
                $r .= '"' . $v . '"';
            }
            else
            {
                $r .= "$v";
            }
        }
    }

    # can and should we flatten it?
    if ( $FLATTEN && $nestLevel && (index($r, "\n") > -1) && (length($r) < (120 + 4 * ($r =~ tr/\n//))) )
    {
        $r =~ s/\s*\n\s*/ /sg;
    }

    return $r;
}


# $string = _stack(traceLevel)
sub _stack
{
    my $traceLevel = shift;
    my $frame = 0;

    if ($traceLevel > 1)
    {
        my @trace = map { s/\t//; s/^\s*//g;
                          s/$PREFIX//g if ($PREFIX);
                          'frame ' . $frame++ . ': ' . $_; } split(/\n/, longmess_heavy());
        return \@trace;
    }
    else
    {
        my $trace = shortmess_heavy();
        return [ substr($trace, 1, index($trace, "\n")-1) ];
    }
}

# stuff below stolen from Carp.pm from Perl 5.8.12.

our $MaxEvalLen = 40;
our $Verbose    = 0;
our $CarpLevel  = 0;
our $MaxArgLen  = 64;   # How much of each argument to print. 0 = all.
our $MaxArgNums = 8;    # How many arguments to print. 0 = all.
our %Internal = ();
our %CarpInternal = ( 'Ffi::Debug' => 1 );

our @CARP_NOT = qw( );


sub longmess_heavy {
  return @_ if ref($_[0]); # don't break references as exceptions
  my $i = long_error_loc();
  return ret_backtrace($i, @_);
}

sub shortmess_heavy {
  return longmess_heavy(@_) if $Verbose;
  return @_ if ref($_[0]); # don't break references as exceptions
  my $i = short_error_loc();
  if ($i) {
    ret_summary($i, @_);
  }
  else {
    longmess_heavy(@_);
  }
}

sub short_error_loc {
  # You have to create your (hash)ref out here, rather than defaulting it
  # inside trusts *on a lexical*, as you want it to persist across calls.
  # (You can default it on $_[2], but that gets messy)
  my $cache = {};
  my $i = 1;
  my $lvl = $CarpLevel;
  {

    my $called = defined &{"CORE::GLOBAL::caller"} ? &{"CORE::GLOBAL::caller"}($i) : caller($i);
    $i++;
    my $caller = defined &{"CORE::GLOBAL::caller"} ? &{"CORE::GLOBAL::caller"}($i) : caller($i);

    return 0 unless defined($caller); # What happened?
    redo if $Internal{$caller};
    redo if $CarpInternal{$caller};
    redo if $CarpInternal{$called};
    redo if trusts($called, $caller, $cache);
    redo if trusts($caller, $called, $cache);
    redo unless 0 > --$lvl;
  }
  return $i - 1;
}


# Returns a full stack backtrace starting from where it is
# told.
sub ret_backtrace {
  my ($i, @error) = @_;
  my $mess;
  my $err = join '', @error;
  $i++;

  my $tid_msg = '';
  if (defined &threads::tid) {
    my $tid = threads->tid;
    $tid_msg = " thread $tid" if $tid;
  }

  my %i = caller_info($i);
  $mess = "$err at $i{file} line $i{line}$tid_msg\n";

  while (my %i = caller_info(++$i)) {
      $mess .= "\t$i{sub_name} called at $i{file} line $i{line}$tid_msg\n";
  }
  
  return $mess;
}

sub caller_info {
  my $i = shift(@_) + 1;
  my %call_info;
  {
  package DB;
  our @args = \$i; # A sentinal, which no-one else has the address of
  @call_info{
    qw(pack file line sub has_args wantarray evaltext is_require)
  } = defined &{"CORE::GLOBAL::caller"} ? &{"CORE::GLOBAL::caller"}($i) : caller($i);
  }
  
  unless (defined $call_info{pack}) {
    return ();
  }

  my $sub_name = get_subname(\%call_info);
  if ($call_info{has_args}) {
    my @args;
    if (@DB::args == 1 && ref $DB::args[0] eq ref \$i && $DB::args[0] == \$i) {
      @DB::args = (); # Don't let anyone see the address of $i
      @args = "** Incomplete caller override detected; \@DB::args were not set **";
    } else {
      @args = map {format_arg($_)} @DB::args;
    }
    if ($MaxArgNums and @args > $MaxArgNums) { # More than we want to show?
      $#args = $MaxArgNums;
      push @args, '...';
    }
    # Push the args onto the subroutine
    $sub_name .= '(' . join (', ', @args) . ')';
  }
  $call_info{sub_name} = $sub_name;
  return wantarray() ? %call_info : \%call_info;
}

# Takes an inheritance cache and a package and returns
# an anon hash of known inheritances and anon array of
# inheritances which consequences have not been figured
# for.
sub get_status {
    my $cache = shift;
    my $pkg = shift;
    $cache->{$pkg} ||= [{$pkg => $pkg}, [trusts_directly($pkg)]];
    return @{$cache->{$pkg}};
}


# Takes the info from caller() and figures out the name of
# the sub/require/eval
sub get_subname {
  my $info = shift;
  if (defined($info->{evaltext})) {
    my $eval = $info->{evaltext};
    if ($info->{is_require}) {
      return "require $eval";
    }
    else {
      $eval =~ s/([\\\'])/\\$1/g; $eval =~ s/\n/ /g;
      return "eval '" . str_len_trim($eval, $MaxEvalLen) . "'";
    }
  }

  return ($info->{sub} eq '(eval)') ? 'eval {...}' : $info->{sub};
}

# Transform an argument to a function into a string.
sub format_arg {
  my $arg = shift;
  if (ref($arg)) {
      $arg = defined($overload::VERSION) ? overload::StrVal($arg) : "$arg";
  }
  if (defined($arg)) {
      $arg =~ s/'/\\'/g;
      $arg = str_len_trim($arg, $MaxArgLen);
  
      # Quote it?
      $arg = "'$arg'" unless $arg =~ /^-?[\d.]+\z/;
  } else {
      $arg = 'undef';
  }
  # The following handling of "control chars" is direct from
  # the original code - it is broken on Unicode though.
  # Suggestions?
  utf8::is_utf8($arg)
    or $arg =~ s/([[:cntrl:]]|[[:^ascii:]])/sprintf("\\x{%x}",ord($1))/eg;
  return $arg;
}



sub ret_summary {
  my ($i, @error) = @_;
  my $err = join '', @error;
  $i++;

  my $tid_msg = '';
  if (defined &threads::tid) {
    my $tid = threads->tid;
    $tid_msg = " thread $tid" if $tid;
  }

  my %i = caller_info($i);
  return "$err at $i{file} line $i{line}$tid_msg\n";
}
sub long_error_loc {
  my $i;
  my $lvl = $CarpLevel;
  {
    ++$i;
    my $pkg = defined &{"CORE::GLOBAL::caller"} ? &{"CORE::GLOBAL::caller"}($i) : caller($i);
    unless(defined($pkg)) {
      # This *shouldn't* happen.
      if (%Internal) {
        local %Internal;
        $i = long_error_loc();
        last;
      }
      else {
        # OK, now I am irritated.
        return 2;
      }
    }
    redo if $CarpInternal{$pkg};
    redo unless 0 > --$lvl;
    redo if $Internal{$pkg};
  }
  return $i - 1;
}


# If a string is too long, trims it with ...
sub str_len_trim {
  my $str = shift;
  my $max = shift || 0;
  if (2 < $max and $max < length($str)) {
    substr($str, $max - 3) = '...';
  }
  return $str;
}

# Takes two packages and an optional cache.  Says whether the
# first inherits from the second.
#
# Recursive versions of this have to work to avoid certain
# possible endless loops, and when following long chains of
# inheritance are less efficient.
sub trusts {
    my $child = shift;
    my $parent = shift;
    my $cache = shift;
    my ($known, $partial) = get_status($cache, $child);
    # Figure out consequences until we have an answer
    while (@$partial and not exists $known->{$parent}) {
        my $anc = shift @$partial;
        next if exists $known->{$anc};
        $known->{$anc}++;
        my ($anc_knows, $anc_partial) = get_status($cache, $anc);
        my @found = keys %$anc_knows;
        @$known{@found} = ();
        push @$partial, @$anc_partial;
    }
    return exists $known->{$parent};
}

# Takes a package and gives a list of those trusted directly
sub trusts_directly {
    my $class = shift;
    no strict 'refs';
    no warnings 'once'; 
    return @{"$class\::CARP_NOT"}
      ? @{"$class\::CARP_NOT"}
      : @{"$class\::ISA"};
}




=pod

=head2 See also

L<Ffi>


=cut

###############################################################################

1;
# eof

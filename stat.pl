# accurate mean, std-dev with '-x' option
# assumed all numbers are valid (hex-float ok)

if ($ARGV[0] eq '-x') {
    $cmd = '0 [ m sqr + ] r{ x r sqr - r{ / r r{ / r{';
    $pipe = '<' . pop @ARGV if $ARGV[1];
    ($M2, $mean, $n) = `rpn $cmd $pipe`;
    goto DONE;
}

binmode STDIN;
while (<>) {                  # Welford's method
    if ($x = $_ + 0) {
        $delta = $x - $mean;
        $mean += $delta / ++$n;
        $M2 += $delta * ($x - $mean);
    } elsif (/^[ \t]*[+-]?[.]?[0-9]/) {
        $x = $mean;
        $mean -= $x / ++$n;   # add valid zero
        $M2 += $x * $mean;
    }
}

DONE:
printf "Items\t%.f\n", $n;
exit if $n == 0;
printf "Average\t%.17g\n", $mean;
exit if $n == 1;
printf "Std Dev\t%.17g\n", sqrt($M2 / ($n-1));

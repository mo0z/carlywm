#!/bin/sh
#
# Fetches info and pipes into a panel. Used to test EWMH compliance of my WM.
#
# Most of this is taken from z3bra.org
#

groups() {
    cur=`xprop -root _NET_CURRENT_DESKTOP | awk -F'[, \t]*' '{print $3}'`
    tot=`xprop -root _NET_NUMBER_OF_DESKTOPS | awk '{print $3}'`

    for w in `seq 0 $((cur - 1))`; do line="${line} [ ]"; done
    line="%{B-}${line} %{B#99736E}[$((cur + 1))]%{B-}"
    for w in `seq $((cur + 2)) $tot`; do line="${line} [ ]"; done
    echo -n $line
}

clock() {
    date +%H:%M
}

battery() {
    cat /sys/class/power_supply/BAT0/capacity
}

#nowplaying() {
 #   curr=`mpc current`
  #  test "$1" = "scroll" && PARSER='skroll -n20 -d0.5 -r' || PARSER='cat'
    #  test -n "$cur" && $PARSER <<< $cur || echo "- stopped -"
  #  echo $curr
#}

# Fill buffer, output to STDOUT
while :; do
    buf=""
    buf="${buf}%{l}%{B#99736E} pjm  %{B-}   "
    buf="${buf} %{B-}$(groups)"
    buf="${buf}%{c}%{B-}-[$(clock)]-"
 #   buf="${buf}%{r}%{B#99736E} MPD: $(nowplaying) ----------   "
    buf="${buf}%{r}  %{B-}   BAT: $(battery)%"
    buf="${buf} "
 
    echo $buf
    sleep 0.25
done

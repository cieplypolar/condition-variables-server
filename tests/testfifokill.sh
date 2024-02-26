#!/bin/sh

. test_utils

make || {
	echo Make failed.
	exit 2
}

rm -f out/*

echo Test uczciwosci fifo kiedy przyjdzie sygnal - proces na koniec kolejki
run 1 fifokill  fifokillr.out fifokill.out
run 2 fifokill  fifokillr.out fifokill.out


#!/usr/bin/env bash
function __FILE__ {
    echo ${BASH_SOURCE[0]}
}
file=$(__FILE__)
dirname=${file%/*}

patchfile=${1:-'head'}
case $patchfile in
    1.9.3 | head | trunk )
	for file in \
	    00-extern-access.patch \
	    01-disasm-insns.patch  \
	    02-iseq-access.patch   \
	    03-iseq-create.patch   \
	    04-C-argc.patch        \
	    05-brkpt.patch         #\
	    #06-raise-msg.patch
	do 
	    echo -- Applying patches in ${dirname}/trunk/$file ...
	    patch -p0 < ${dirname}/trunk/$file
	done
	;;
    combined ) 
	file=ruby-trunk-combined.patch
	echo -- Applying patches in ${dirname}/$file ...
	patch -p0 < ${dirname}/$file
	;;
    1.9.2 | rc2  | * )
	file=ruby-1.9.2-combined.patch
	echo -- Applying patches in ${dirname}/$file ...
	patch -p0 < ${dirname}/$file
	;;
    esac

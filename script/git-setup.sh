#!/bin/bash
if [[ $# != 1 ]] ; then
    echo 2>&1 <<EOF 
Usage:
  $0 *ruby-tar.gz*

Untar ruby source and set it up for git.
EOF
    exit 1
fi
tar_file="$1"
if [[ ! -r "$tar_file" ]]; then
    echo "tar file: $tar_file does not exist"  2>&1
    exit 2
fi

tar -xzf $tar_file || {
    echo "Error untarring $ruby_name" 2>&1
    exit 3
}
files="
configure
encdb.h
golf_prelude.c
id.h
insns.inc
insns_info.inc
known_errors.inc
lex.c
miniprelude.c
newline.c
node_name.inc
opt_sc.inc
optinsn.inc
optunifs.inc
parse.c
parse.h
revision.h
transdb.h
vm.inc
vmtc.inc
"

ruby_name=$(basename $tar_file .tar.gz)
git_dir="${ruby_name}-git"
mv -v ${ruby_name} ${git_dir}
cd ${git_dir} || {
    error >2 "Error in cd to ${git_dir}"
    exit 4
}
git init .
rm -fv $files
git add *
git commit -m'What we got to work with' .
exit $?

#!/bin/env zsh

# Locates the compilation database & grooms it so that the WSL clangd LSP can read the paths

# Script needs an argument to run
if [[ $# != 1 ]]; then
    echo "Need a build path & nothing more"
    return -1;
fi

build_path=$1

# Append the search directory to the build folder path
if [[ ${build_path[-1]} = '/' ]]; then
    build_path=$build_path'.qtc_clangd'
else
    build_path=$build_path'/.qtc_clangd'
fi

# Verify the path is valid
win_compilation_database=$build_path'/compile_commands.json'

if ! [[ -f $win_compilation_database ]]; then
    echo "The database aint here"
    return -2;
fi

# Read the contents of the file into an array
contents=( "${(@f)$(<$win_compilation_database)}" )
# Globally substitute all the back slashes to forward slashes
sub_slashes=( ${contents[@]//"\\\\"/'/'} )
# Globally substitute all the Windows paths to mounted windows paths from WSL
wsl_filepath=( ${sub_slashes[@]//'C:'/'/mnt/c'} )

# Write the groomed compilation database to the projects working directory for the LSPs to use
echo $wsl_filepath > 'compile_commands.json'

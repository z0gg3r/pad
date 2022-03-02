# NAME

pad - Pad a string to a given length

# SYNOPSIS

**pad STRING** \[**-l** *LENGTH*\] \[**-c** *CHAR*\] \[**-m** *MODE*\]
\[**-s** *STRING*\]

# DESCRIPTION

**pad** is a small program that uses libpadding to pad a string to a
given length.

# OPTIONS

**-l, \--length LENGTH**

:   sets the length of the result string (Default: 80)

**-c, \--character CHAR**

:   sets the char to use for padding (Default: \' \')

**-m, \--mode MODE**

:   sets the padding mode. Possible values are \"left\", \"right\",
    \"center\" and \"both\" (Default: \"both\")

**-s, \-- STRING**

:   sets the string that you want to pad. Use -s explicitly if you want
    to pad an empty string.

**-h, \--help**

:   show help message

# NOTES

If **pad** is given the mode \"center\" a user specifed length will be
ignored. It also currently does not use the padding library and instead
prints directly. This may change in the future.

# BUGS

**pad** used to be unable to handle non-ASCII padding characters. This
has been fixed, however the actual length of the line with non-ASCII
characters may differ from the desired length.

# AUTHOR

zockerfreunde03/z0gg3r

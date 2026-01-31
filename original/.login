umask 002
setenv TERM `term -c -M ibm`
if($status == 6) stty ibm cr0
set prompt='Hack: '
setenv SCROLL jump

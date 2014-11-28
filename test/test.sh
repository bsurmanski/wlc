tdirs="all construct gltest libvec postdecl switch vararg\
    array crossimport importc logic_exp retain weak cast delete interface\
    overload stack tuple class funcptr labels pack struct union"

for dir in $tdirs; do
    cd $dir
    make &>/dev/null
    err=$?
    if [ $err -gt 0 ]; then
        echo -e "\033[1;31m" $dir " ERROR: " $err "\033[0m"
    else
        echo $dir "SUCCESS"
    fi
    cd ..
done

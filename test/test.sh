tdirs="all construct gltest libvec postdecl switch vararg\
    array coerce crossimport importc logic_exp retain weak cast delete interface\
    overload stack tuple class funcptr labels pack struct static union ufcs virtual"

for dir in $tdirs; do
    cd $dir
    make &>/dev/null
    err=$?

    if [ $err -gt 0 ]; then
        echo -e "\033[1;31m" $dir "COMPILE ERROR: " $err "\033[0m"
    elif [ -e expect.out ]; then
        ./program &> run.out
        if [ -n "$(diff expect.out run.out)" ]; then
            echo -e "\033[1;31m" $dir "INVALID OUTPUT" "\033[0m"
        else
            echo $dir "SUCCESS!"
        fi
    else
        echo $dir "SUCCESS"
    fi
    cd ..
done

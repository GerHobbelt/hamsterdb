
sh ham_info/compare.sh
if [ $? -ne 0 ]; then
    exit 1
fi

sh ham_dump/compare.sh
if [ $? -ne 0 ]; then
    exit 1
fi

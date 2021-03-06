function checkDelete() {
	if ! [ -f $2  ]
	then
		filename=$(basename $1)
		dirname=$(dirname $1)
		rm "$dirname/DCAP_${filename:5}" -i
	fi
}

function checkWarn() {
	if ! [ -f $(readlink -e $2)  ]
	then
		echo "missing: $2 from list $(basename $1)"
	fi
}


# check if files in sample lists still point to an existing file
for list in $CMSSW_BASE/src/HiggsAnalysis/KITHiggsToTauTau/data/Samples/DCAP_sample*.txt;
do
	if ! [ -L $list ]
	then
		testfile=$(head -n 1 $list)
		checkDelete $list ${testfile:31}
	fi
done 

# check if all files in a collection are still present and warn if not
for list in $CMSSW_BASE/src/HiggsAnalysis/KITHiggsToTauTau/data/Samples/DCAP_collection*.txt;
do
	if ! [ -L $list ]
	then
		for line in $(cat $list);
		do
			checkWarn $list $line
		done
	fi
done 
# check if symlinks still point to something
for list in $CMSSW_BASE/src/HiggsAnalysis/KITHiggsToTauTau/data/Samples/DCAP_sample*.txt;
do
	if [ -L $list ]
	then
		if ! readlink -q $list>>/dev/null
		then
			echo "fails: $list"
		fi
	fi
done

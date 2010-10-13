#!/bin/bash
TOOL_DIR=$1
SUB_DIR=$2
while [ 1 ]
do
#[ ! -d " ~/public_html/hunt/$SUB_DIR" ] && echo Create  ~/public_html/hunt/$SUB_DIR && mkdir ~/public_html/hunt/$SUB_DIR
[ ! -d "$SUB_DIR" ] && echo Create $SUB_DIR && mkdir $SUB_DIR
[ ! -d "$SUB_DIR/cands" ] && echo Create $SUB_DIR/cands && mkdir $SUB_DIR/cands
[ ! -d "$SUB_DIR/dumps" ] && echo Create $SUB_DIR/dumps && mkdir $SUB_DIR/dumps
[ ! -d "$SUB_DIR/dumps2xfer" ] && echo Create $SUB_DIR/dumps2xfer && mkdir $SUB_DIR/dumps2xfer
[ ! -d "$SUB_DIR/logs" ] && echo Create $SUB_DIR/logs && mkdir $SUB_DIR/logs
[ ! -d "$SUB_DIR/emu_skim" ] && echo Create $SUB_DIR/emu_skim && mkdir $SUB_DIR/emu_skim
[ ! -d "$SUB_DIR/emu_baby" ] && echo Create $SUB_DIR/emu_baby && mkdir $SUB_DIR/emu_baby
[ ! -d "$SUB_DIR/dilep_skim" ] && echo Create $SUB_DIR/dilep_skim && mkdir $SUB_DIR/dilep_skim
[ ! -d "$SUB_DIR/dilep_baby" ] && echo Create $SUB_DIR/dilep_baby && mkdir $SUB_DIR/dilep_baby
[ ! -d "$SUB_DIR/trilep_skim" ] && echo Create $SUB_DIR/trilep_skim && mkdir $SUB_DIR/trilep_skim
[ ! -d "$SUB_DIR/trilep_baby" ] && echo Create $SUB_DIR/trilep_baby && mkdir $SUB_DIR/trilep_baby
rm  $SUB_DIR/AllRunsAvailable.txt
touch $SUB_DIR/AllRunsAvailable.txt
cp rootrc $SUB_DIR/.rootrc
cp rootlogon.C $SUB_DIR/.
cd $SUB_DIR
    # get time stamp
    DATE=`date +%Y%m%d%H%M%S`
    echo $DATE

    echo "Checking for new files to process..."
    #find /nfs-3/userdata/cms2/EG_Run2010A-PromptReco-v4_RECO/V03-04-25/singleLepPt5Skim -follow -maxdepth 1 -name "skimmed_ntuple_[0-9]*.root" -printf "%p %s %C@\n"  >AllRunsAvailable.txt
    echo /nfs-3/userdata/cms2/$SUB_DIR/V03-06-09/singleLepPt10Skim
    find /nfs-3/userdata/cms2/$SUB_DIR/V03-06-09/singleLepPt10Skim -follow -maxdepth 1 -name "skimmed_ntuple_[0-9]*.root" -printf "%p %s %C@\n"  >>  AllRunsAvailable.txt
#     find /nfs-3/userdata/cms2/Mu_Run2010A-PromptReco-v4_RECO/V03-04-26-12/singleLepPt10Skim -follow -maxdepth 1 -name "skimmed_ntuple_[0-9]*.root" -printf "%p %s %C@\n"  >AllRunsAvailable.txt
    # The awk above is used because these two processings overlap
    # and we don't want to deal with the duplication
    # The second processing below is for runs >= 138728
   # find /nfs-3/userdata/cms2/MinimumBias_Commissioning10-SD_Mu-Jun14thSkim_v1_RECO/V03-04-26-02/singleLepPt10Skim -follow -maxdepth 1 -name "skimmed_ntuple_[0-9]*.root" -printf "%p %s %C@\n" | awk -F'_' '{if ($6<135446) print $0}'  >>AllRunsAvailable.txt
    rm RunsToProcess.txt
    touch RunsProcessed.txt

    # loop over all AllRunsAvailable.txt, check RunsProcessed.txt,
    # if not there, add to RunsToProcess.txt
    while read line
    do
        grep "$line" RunsProcessed.txt >/dev/null 2>&1
        if [ $? -ne 0 ]
        then
            run=`echo $line | awk -F'_' '{print $5}'`
            if [ $run -gt 133222 ] && [ $run -le 133250 ];
            then
                echo "Magnetic field off, skipping run "$run
            else 
                echo $line >> RunsToProcess.txt
            fi
        fi
    done < AllRunsAvailable.txt

    # check if there are new files and process them gods willing
    nfiles=`cat RunsToProcess.txt | wc -l`
    if [ $nfiles -lt 1 ]
    then
        echo "No new runs to process..."
    else
        while read line
        do
            file=`echo $line | awk '{print $1}'`
            cmd="root -b -l -q '$TOOL_DIR/UpdateSkims.C+(\"$file\")'"
            eval $cmd
            if [ $? -ne 0 ]
            then
                echo "Error processing $file"
            else
                echo $line >> RunsProcessed.txt
            fi
        done <RunsToProcess.txt

        # slava's [re-]merging may remove files in which case we
        # will have duplicates so rm parentless babies and skims
        echo "Cleaning out parentless babies and skims"
        # emu
        for baby in `ls emu_baby/*.root`
        do
            ident=`echo $baby | sed 's!^.*/emuskim_baby_\(.*\).root$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless baby, removing $baby"
                rm $baby
            fi
        done
        for skim in `ls emu_skim/*.root`
        do
            ident=`echo $skim | sed 's!^.*/emuskim_\(.*\).root$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless skim, removing $skim"
                rm $skim
            fi
        done
        # dilep
        for baby in `ls dilep_baby/*.root`
        do
            ident=`echo $baby | sed 's!^.*/dilepskim_baby_\(.*\).root$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless baby, removing $baby"
                rm $baby
            fi
        done
        for skim in `ls dilep_skim/*.root`
        do
            ident=`echo $skim | sed 's!^.*/dilepskim_\(.*\).root$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless skim, removing $skim"
                rm $skim
            fi
        done
        # trilep
        for baby in `ls trilep_baby/*.root`
        do
            ident=`echo $baby | sed 's!^.*/trilepskim_baby_\(.*\).root$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless baby, removing $baby"
                rm $baby
            fi
        done
        for skim in `ls trilep_skim/*.root`
        do
            ident=`echo $skim | sed 's!^.*/trilepskim_\(.*\).root$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless skim, removing $skim"
                rm $skim
            fi
        done
        # clean logs as well
        for log in `ls logs/*.txt`
        do
            ident=`echo $log | sed 's!^.*/log_\(.*\).txt$!\1!'`
            grep $ident AllRunsAvailable.txt >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                echo "Found parentless log, removing $log"
                rm $log
            fi
        done

        # Produce cands.txts and do some basic plotting
        # in a separate process so that if it stalls it
        # it does not affect this process
        
	
        #source $TOOL_DIR/makeCands.sh $TOOL_DIR $SUB_DIR&

        # Last run processed
        #cat RunsProcessed.txt | cut -d '_' -f 6 | sort -n -r | head -n 1 >lastrun.txt
	cat RunsProcessed.txt | cut -d '_' -f 5 | sort -n -r | head -n 1 >lastrun.txt
        #scp lastrun.txt uaf-4.t2.ucsd.edu:~/public_html/hunt
	#cp lastrun.txt ~/public_html/hunt/$SUB_DIR
    fi
    cd $TOOL_DIR
    sleep 600;
done  &> $TOOL_DIR/log/$SUB_DIR.log.`date '+%Y.%m.%d-%H.%M.%S'`   
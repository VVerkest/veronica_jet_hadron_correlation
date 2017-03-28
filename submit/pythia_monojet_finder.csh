    #!/bin/csh

    # first make sure program is updated and exists
    make bin/pythia_monojet_finder || exit

    set analysis = 'ppjet'
    set command = './bin/pythia_monojet_finder'
    set base = AddedGeantPythia/

    if ( $# != "3" && !( $2 == 'default' ) ) then
    echo 'Error: illegal number of parameters'
    exit
    endif
    
    # Start the Condor File
    echo "" > CondorFile
    echo "Universe    = vanilla" >> CondorFile
    echo "Executable  = ${command}" >> CondorFile
    echo "getenv = true" >> CondorFile
	
    # Arguments
    set leadPtMin = $1
    set jetPtMax = $2
    set jetRadius = $3

    if ( $2 == 'default' ) then
    set jetPtMax = 100.0
    set jetRadius = 0.4
    endif

    # Create the folder name for output
    set outFile = pythia_${analysis}_lead_${leadPtMin}_max_${jetPtMax}_rad_${jetRadius}
    # Make the directories since they may not exist...
    if ( ! -d out/${outFile} ) then
    mkdir -p out/${outFile}
    mkdir -p out/${outFile}/correlations
    mkdir -p out/${outFile}/tree
    mkdir -p out/${outFile}/mixing
    endif

    if ( ! -d log/${outFile} ) then
    mkdir -p log/${outFile}
    endif

    # Now Submit jobs for each data file
    foreach input ( ${base}* )

    # Create the output file base name
    set OutBase = `basename $input | sed 's/.root//g'`

    # Make the output names and path
    set outLocation = "out/${outFile}/"
    set outName = correlations/corr_${OutBase}.root
    set outNameTree = tree/tree_${OutBase}.root

    # Input files
    set Files = ${input}

    # Logfiles. Thanks cshell for this "elegant" syntax to split err and out
    set LogFile     = log/${outFile}/${analysis}_${OutBase}.out
    set ErrFile     = log/${outFile}/${analysis}_${OutBase}.err

    echo "Logging output to " $LogFile
    echo "Logging errors to " $ErrFile

    set arg = "$leadPtMin $jetPtMax $outLocation $outName $outNameTree $Files"
    set execute = "${command} ${arg}"
    
    # Write to CondorFile
    echo "Executing " $execute

    echo " " >> CondorFile
    echo "Output    = ${LogFile}" >> CondorFile
    echo "Error     = ${ErrFile}" >> CondorFile
    echo "Arguments = ${arg}" >> CondorFile
    echo "Queue" >> CondorFile


    end

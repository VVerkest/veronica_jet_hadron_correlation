#!/bin/csh

# first make sure program is updated and exists
make bin/pythia_monojet_finder || exit

set analysis = $1
set command = './bin/pythia_monojet_finder'
set base = AddedGeantPythia/

if ( $# != "7" && !( $2 == 'default' ) ) then
echo 'Error: illegal number of parameters'
exit
endif

echo $analysis
if ( $analysis != 'ppdijet' && $analysis != 'ppjet' ) then
echo 'Error: unknown analysis type'
exit
endif

# Start the Condor File
echo "" > CondorFile
echo "Universe    = vanilla" >> CondorFile
echo "Executable  = ${command}" >> CondorFile
echo "getenv = true" >> CondorFile

# Arguments
set useEfficiency = $2
set triggerCoincidence = $3
set subLeadPtMin = $4
set leadPtMin = $5
set jetPtMax = $6
set jetRadius = $7

if ( $2 == 'default' ) then
set useEfficiency = 'false'
set triggerCoincidence = 'false'
if ( $analysis == 'ppdijet' ) then
set subLeadPtMin = 10.0
set leadPtMin = 20.0
set jetPtMax = 100.0
else if ( $analysis == 'ppjet' ) then
set subLeadPtMin = 0.0
set leadPtMin = 20.0
set jetPtMax = 100.0
endif
endif
set jetRadius = 0.4
endif

# Create the folder name for output
set outFile = pythia_${analysis}_trigger_${triggerCoincidence}_eff_${useEfficiency}_lead_${leadPtMin}_sub_${subLeadPtMin}_max_${jetPtMax}_rad_${jetRadius}
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

set arg = "$analysis $useEfficiency $triggerCoincidence $subLeadPtMin $leadPtMin $jetPtMax $jetRadius $outLocation $outName $outNameTree $Files $mbData"
set execute = "${command} ${arg}"

# Write to CondorFile
echo "Executing " $execute

echo " " >> CondorFile
echo "Output    = ${LogFile}" >> CondorFile
echo "Error     = ${ErrFile}" >> CondorFile
echo "Arguments = ${arg}" >> CondorFile
echo "Queue" >> CondorFile


end

#!/bin/bash
#SBATCH -J CellML_Test
#SBATCH -A uoa00322
#SBATCH --time=00:30:00     # Walltime
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=2
#SBATCH --mem-per-cpu=1024  # memory/cpu (in MB)
#SBATCH -o lowmutzerox_%j.out       # OPTIONAL
#SBATCH -e lowmutzerox_%j.err       # OPTIONAL
#SBATCH -C sb
######################################################

module load intel/ics-2013 

# User-configured paths
######################################################
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../cellml-sdk/lib/
export LIBRARY_PATH=$LIBRARY_PATH:../../../cellml-sdk/lib/
######################################################

srun ../../cellml-fitter lowmutzerox.xml -v -v -v -v  

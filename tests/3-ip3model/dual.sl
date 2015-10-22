#!/bin/bash
#SBATCH -J Cel-dual
#SBATCH -A uoa00322
#SBATCH --time=00:05:00     # Walltime
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1024  # memory/cpu (in MB)
#SBATCH -o exper_cellml_%j.out       # OPTIONAL
#SBATCH -e exper_cellml_%j.err       # OPTIONAL
#SBATCH -C sb
######################################################

module load intel/ics-2013 

# User-configured paths
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../cellml-sdk/lib/	
export LIBRARY_PATH=$LIBRARY_PATH:../../../cellml-sdk/lib/

srun ../../cellml-fitter dual.xml -v -v -v -v

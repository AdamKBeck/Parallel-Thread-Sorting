BATCH --nodes=1
#SBATCH --cpus-per-task=16
#SBATCH --output=final_output.txt
#SBATCH --time=0-00:01:00

make
cp ParallelSorting $PFSDIR/.
cd $PFSDIR
./ParallelSorting

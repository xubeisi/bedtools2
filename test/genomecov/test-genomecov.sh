set -e;
BT=${BT-../../bin/bedtools}

FAILURES=0;

check()
{
	if diff $1 $2; then
    	echo ok
	else
    	FAILURES=$(expr $FAILURES + 1);
		echo fail
	fi
}

###########################################################
###########################################################
#                       BAM files                         #
###########################################################
###########################################################
samtools view -Sb one_block.sam > one_block.bam 2>/dev/null
samtools view -Sb two_blocks.sam > two_blocks.bam 2>/dev/null
samtools view -Sb three_blocks.sam > three_blocks.bam 2>/dev/null
samtools view -Sb sam-w-del.sam > sam-w-del.bam 2>/dev/null
samtools view -Sb pair-chip.sam > pair-chip.bam 2>/dev/null
samtools view -Sb chip.sam > chip.bam 2>/dev/null
samtools view -Sb chip2.sam > chip2.bam 2>/dev/null



##################################################################
#  Test three blocks without -split
##################################################################
echo -e "    genomecov.t1...\c"
echo \
"chr1	0	50	1" > exp
$BT genomecov -ibam three_blocks.bam -bg > obs
check obs exp
rm obs exp


##################################################################
#  Test three blocks with -split
##################################################################
echo -e "    genomecov.t2...\c"
echo \
"chr1	0	10	1
chr1	20	30	1
chr1	40	50	1" > exp
$BT genomecov -ibam three_blocks.bam -bg -split > obs
check obs exp
rm obs exp


##################################################################
#  Test three blocks with -split and -bga
##################################################################
echo -e "    genomecov.t3...\c"
echo \
"chr1	0	10	1
chr1	10	20	0
chr1	20	30	1
chr1	30	40	0
chr1	40	50	1
chr1	50	1000	0" > exp
$BT genomecov -ibam three_blocks.bam -bga -split > obs
check obs exp
rm obs exp


##################################################################
#  Test blocked BAM from multiple files w/ -bga and w/o -split
##################################################################
echo -e "    genomecov.t4...\c"
echo \
"chr1	0	30	3
chr1	30	40	2
chr1	40	50	1
chr1	50	1000	0" > exp
samtools merge -f /dev/stdout *block*.bam | $BT genomecov -ibam - -bga  > obs
check obs exp
rm obs exp


##################################################################
#  Test blocked BAM from multiple files w/ -bga and w -split
##################################################################
echo -e "    genomecov.t5...\c"
echo \
"chr1	0	10	3
chr1	10	15	2
chr1	15	20	1
chr1	20	25	2
chr1	25	30	3
chr1	30	50	1
chr1	50	1000	0" > exp
samtools merge -f /dev/stdout *block*.bam | $BT genomecov -ibam - -bga -split > obs
check obs exp
rm obs exp


##################################################################
#  Test three blocks with -split and -dz
##################################################################
echo -e "    genomecov.t6...\c"
echo \
"chr1	0	1
chr1	1	1
chr1	2	1
chr1	3	1
chr1	4	1
chr1	5	1
chr1	6	1
chr1	7	1
chr1	8	1
chr1	9	1
chr1	20	1
chr1	21	1
chr1	22	1
chr1	23	1
chr1	24	1
chr1	25	1
chr1	26	1
chr1	27	1
chr1	28	1
chr1	29	1
chr1	40	1
chr1	41	1
chr1	42	1
chr1	43	1
chr1	44	1
chr1	45	1
chr1	46	1
chr1	47	1
chr1	48	1
chr1	49	1" > exp
$BT genomecov -ibam three_blocks.bam -dz -split > obs
check obs exp
rm obs exp


##################################################################
#  Test SAM with 1bp D operator
##################################################################
echo -e "    genomecov.t7...\c"
echo \
"chr1	0	10	1
chr1	11	21	1" > exp
$BT genomecov -ibam sam-w-del.bam -bg > obs
check obs exp
rm obs exp

##################################################################
#  Test bam with chroms that have no coverage
##################################################################
echo -e "    genomecov.t8...\c"
echo \
"1	0	93	100	0.93
1	1	4	100	0.04
1	2	3	100	0.03
2	0	100	100	1
3	0	100	100	1
genome	0	293	300	0.976667
genome	1	4	300	0.0133333
genome	2	3	300	0.01" > exp
$BT genomecov -ibam y.bam > obs
check obs exp
rm obs exp

##################################################################
#  Test bam with chroms that have no coverage
##################################################################
echo -e "    genomecov.t9...\c"
echo \
"1	15	17	1
1	17	20	2
1	20	22	1" > exp
$BT genomecov -ibam y.bam -bg > obs
check obs exp
rm obs exp

##################################################################
#  Test bam with chroms that have no coverage
##################################################################
echo -e "    genomecov.t10...\c"
echo \
"1	0	15	0
1	15	17	1
1	17	20	2
1	20	22	1
1	22	100	0
2	0	100	0
3	0	100	0" > exp
$BT genomecov -ibam y.bam -bga > obs
check obs exp
rm obs exp

##################################################################
#  Test bed with chroms that have no coverage
##################################################################
echo -e "    genomecov.t11...\c"
echo \
"chr1	0	391	400	0.9775
chr1	1	4	400	0.01
chr1	2	3	400	0.0075
chr1	3	2	400	0.005
chr2	0	93	100	0.93
chr2	1	1	100	0.01
chr2	3	6	100	0.06
chr3	0	86	100	0.86
chr3	1	14	100	0.14
chr4	0	100	100	1
genome	0	670	700	0.957143
genome	1	19	700	0.0271429
genome	2	3	700	0.00428571
genome	3	8	700	0.0114286" > exp
$BT genomecov -i y.bed -g genome.txt > obs
check obs exp
rm obs exp

##################################################################
#  Test bed with chroms that have no coverage
##################################################################
echo -e "    genomecov.t12...\c"
echo \
"chr1	15	17	1
chr1	17	18	2
chr1	18	20	3
chr1	20	22	2
chr1	22	24	1
chr2	27	28	1
chr2	28	34	3
chr3	20	34	1" > exp
$BT genomecov -i y.bed -g genome.txt -bg > obs
check obs exp
rm obs exp

##################################################################
#  Test bed with chroms that have no coverage
##################################################################
echo -e "    genomecov.t13...\c"
echo \
"chr1	0	15	0
chr1	15	17	1
chr1	17	18	2
chr1	18	20	3
chr1	20	22	2
chr1	22	24	1
chr1	24	400	0
chr2	0	27	0
chr2	27	28	1
chr2	28	34	3
chr2	34	100	0
chr3	0	20	0
chr3	20	34	1
chr3	34	100	0
chr4	0	100	0" > exp
$BT genomecov -i y.bed -g genome.txt -bga > obs
check obs exp
rm obs exp

##################################################################
#  Test pair-end chip 
##################################################################
echo -e "    genomecov.t14...\c"
echo \
"chr1	0	203	1" > exp
$BT genomecov -ibam pair-chip.bam -bg -pc > obs
check obs exp
rm obs exp

##################################################################
#  Test chip fragmentSize
##################################################################
echo -e "    genomecov.t15 -bg -fs 5...\c"
echo \
"chr1	1	6	1
chr1	295	300	1" > exp
$BT genomecov -ibam chip.bam -bg -fs 5 > obs
check obs exp
rm obs exp

echo -e "    genomecov.t16 -bg -fs 5:3 ...\c"
echo \
"chr1	2	5	1
chr1	296	299	1" > exp
$BT genomecov -ibam chip.bam -bg -fs 5:3 > obs
check obs exp
rm obs exp

echo -e "    genomecov.t17 -bg -fs 5:3:4 ...\c"
echo \
"chr1	2	5	1
chr1	297	300	1" > exp
$BT genomecov -ibam chip.bam -bg -fs 5:3:4 > obs
check obs exp
rm obs exp

echo -e "    genomecov.t18 -bg -fs 5:3:4:2 ...\c"
echo \
"chr1	2	5	1
chr1	297	299	1" > exp
$BT genomecov -ibam chip.bam -bg -fs 5:3:4:2 > obs
check obs exp
rm obs exp





##################################################################
#  Make sure empty bam doesn't cause failure
##################################################################
echo -e "    genomecov.t19...\c"
echo \
"1	0	100	100	1
2	0	100	100	1
3	0	100	100	1
genome	0	300	300	1" > exp
$BT genomecov -ibam empty.bam > obs
check obs exp
rm obs exp

##################################################################
#  Test order by genome
##################################################################
echo -e "    genomecov.t20 -o  'bed file order by genome' ...\c"
echo \
"chr3	20	34	1
chr2	27	28	1
chr2	28	34	3
chr1	15	17	1
chr1	17	18	2
chr1	18	20	3
chr1	20	22	2
chr1	22	24	1" > exp
$BT genomecov -i y.bed -g genome.txt -bg -o > obs
check obs exp
rm obs exp

echo -e "    genomecov.t21 -o  'bam file order by genome' ...\c"
echo \
"chr3	21	96	1
chr1	1	76	1
chr1	225	300	1" > exp
$BT genomecov -ibam chip2.bam -g genome.txt -bg -o > obs
check obs exp
rm obs exp

rm one_block.bam two_blocks.bam three_blocks.bam sam-w-del.bam pair-chip.bam chip.bam chip2.bam

[[ $FAILURES -eq 0 ]] || exit 1;

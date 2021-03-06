/*****************************************************************************
genomeCoverage.cpp

(c) 2009 - Aaron Quinlan
Hall Laboratory
Department of Biochemistry and Molecular Genetics
University of Virginia
aaronquinlan@gmail.com

Licenced under the GNU General Public License 2.0 license.
******************************************************************************/
#include "lineFileUtilities.h"
#include "genomeCoverageBed.h"


BedGenomeCoverage::BedGenomeCoverage(string bedFile, string genomeFile,
                                     bool eachBase, bool startSites,
                                     bool bedGraph, bool bedGraphAll,
                                     int max, float scale,
                                     bool bamInput, bool obeySplits,
                                     bool filterByStrand, string requestedStrand,
                                     bool only_5p_end, bool only_3p_end,
                                     bool pair_chip, bool haveSize, string fragmentSize, bool dUTP,
                                     bool eachBaseZeroBased, bool add_gb_track_line,
                                     bool orderchrom, string gb_track_line_opts) {

    _bedFile = bedFile;
    _genomeFile = genomeFile;
    _eachBase = eachBase;
    _eachBaseZeroBased = eachBaseZeroBased;
    _startSites = startSites;
    _bedGraph = bedGraph;
    _bedGraphAll = bedGraphAll;
    _max = max;
    _scale = scale;
    _bamInput = bamInput;
    _obeySplits = obeySplits;
    _filterByStrand = filterByStrand;
    _requestedStrand = requestedStrand;
    _only_3p_end = only_3p_end;
    _only_5p_end = only_5p_end;
    _pair_chip_ = pair_chip;
    _haveSize = haveSize;
    _fragmentSize_ext = fragmentSize;
    _dUTP = dUTP;
    _orderchrom = orderchrom;
    _add_gb_track_line = add_gb_track_line;
    _gb_track_line_opts = gb_track_line_opts;
    _currChromName = "";
    _currChromSize = 0 ;
    vector<string> _fragmentSizefield;

    Tokenize(_fragmentSize_ext,_fragmentSizefield,':');

    _fragmentSize = atoi(_fragmentSizefield[0].c_str());
    if (_fragmentSizefield.size() > 1){
        _fragcenter = atof(_fragmentSizefield[1].c_str())/2;
    } else { _fragcenter = -2; }
 
    if (_fragmentSizefield.size() > 2){
        _fragmentSize_rev = atoi(_fragmentSizefield[2].c_str());
    } else { _fragmentSize_rev = _fragmentSize; }

    if (_fragmentSizefield.size() > 3){
        _fragcenter_rev = atof(_fragmentSizefield[3].c_str())/2;
    } else { _fragcenter_rev = _fragcenter; }

    if (_bamInput == false) {
        _genome = new GenomeFile(_genomeFile);
    }

    PrintTrackDefinitionLine();

    if (_bamInput == false) {
        _bed = new BedFile(bedFile);
        CoverageBed();
    }
    else {
        CoverageBam(_bedFile);
    }
}

string BedGenomeCoverage::gettmpfile(const string &chrom) {
    chromToFiles::const_iterator chromIt = _orderchrom_tmp_files.find(chrom);
    if (chromIt != _orderchrom_tmp_files.end())
        return chromIt->second;
    else
        return "";  // chrom not found.
}

void BedGenomeCoverage::PrintTrackDefinitionLine()
{
    //Print Track Definition line (if requested)
    if ( (_bedGraph||_bedGraphAll) && _add_gb_track_line) {
        string line = "track type=bedGraph";
        if (!_gb_track_line_opts.empty()) {
            line += " " ;
            line += _gb_track_line_opts ;
        }
        cout << line << endl;
    }

}


BedGenomeCoverage::~BedGenomeCoverage(void) {
    if (_bamInput == false) {
        delete _bed;
        delete _genome;
    }
}


void BedGenomeCoverage::ResetChromCoverage() {
    _currChromName = "";
    _currChromSize = 0 ;
    std::vector<DEPTH>().swap(_currChromCoverage);
}


void BedGenomeCoverage::StartNewChrom(const string& newChrom) {
    // If we've moved beyond the first encountered chromosomes,
    // process the results of the previous chromosome
    if (_currChromName.length() > 0) {
        ReportChromCoverage(_currChromCoverage, _currChromSize,
                _currChromName, _currChromDepthHist);
    }

    // empty the previous chromosome and reserve new
    std::vector<DEPTH>().swap(_currChromCoverage);

    if (_visitedChromosomes.find(newChrom) != _visitedChromosomes.end()) {
        cerr << "Input error: Chromosome " << _currChromName
             << " found in non-sequential lines. This suggests that the input file is not sorted correctly." << endl;

    }
    _visitedChromosomes.insert(newChrom);

    _currChromName = newChrom;

    // get the current chrom size and allocate space
    _currChromSize = _genome->getChromSize(_currChromName);

    if (_currChromSize >= 0)
        _currChromCoverage.resize(_currChromSize);
    else {
        cerr << "Input error: Chromosome " << _currChromName << " found in your input file but not in your genome file." << endl;
        exit(1);
    }
}


void BedGenomeCoverage::AddCoverage(CHRPOS start, CHRPOS end) {
    // process the first line for this chromosome.
    // make sure the coordinates fit within the chrom
    if (start < _currChromSize)
        _currChromCoverage[start].starts++;
    if (end >= 0 && end < _currChromSize)
        _currChromCoverage[end].ends++;
    else
        _currChromCoverage[_currChromSize-1].ends++;
}


void BedGenomeCoverage::AddBlockedCoverage(const vector<BED> &bedBlocks) {
    vector<BED>::const_iterator bedItr = bedBlocks.begin();
    vector<BED>::const_iterator bedEnd = bedBlocks.end();
    for (; bedItr != bedEnd; ++bedItr) {
        // the end - 1 must be done because BamAncillary::getBamBlocks
        // returns ends uncorrected for the genomeCoverageBed data structure.
        // ugly, but necessary.
        AddCoverage(bedItr->start, bedItr->end - 1);
    }
}


void BedGenomeCoverage::CoverageBed() {

    BED a;

    ResetChromCoverage();

    _bed->Open();
    while (_bed->GetNextBed(a)) {
        if (_bed->_status == BED_VALID) {
            if (_filterByStrand == true) {
                if (a.strand.empty()) {
                    cerr << "Input error: Interval is missing a strand value on line " << _bed->_lineNum << "." <<endl;
                    exit(1);
                }
                if ( ! (a.strand == "-" || a.strand == "+") ) {
                    cerr << "Input error: Invalid strand value (" << a.strand << ") on line " << _bed->_lineNum << "." << endl;
                    exit(1);
                }
                // skip if the strand is not what the user requested.
                if (a.strand != _requestedStrand)
                    continue;
            }

            // are we on a new chromosome?
            if (a.chrom != _currChromName)
                StartNewChrom(a.chrom);

            CHRPOS ext_start = a.start;
            CHRPOS ext_end = a.end;
            if (_haveSize) {
                if (_fragmentSize_rev > 0 && a.strand == "-") {
                    if(ext_end<_fragmentSize_rev) { //sometimes fragmentSize is bigger :(
                        ext_start = 0;
                    } else {
                        ext_start = ext_end - _fragmentSize_rev;
                    }
                } else if (_fragmentSize > 0 && a.strand != "-") {
                    ext_end = ext_start + _fragmentSize;
                }
            }
            if (_fragcenter_rev > -1 && a.strand == "-"){
                float mid = float((float(ext_start) + float(ext_end) + 1)/2);
                if (mid<_fragcenter_rev) {
                    ext_start = 0;
                } else {
                    ext_start = int(mid - _fragcenter_rev);
                }
                ext_end = int(mid + _fragcenter_rev);
            } else if (_fragcenter > -1 && a.strand != "-"){
                float mid = float((float(ext_start) + float(ext_end) + 1)/2);
                if (mid<_fragcenter) {
                    ext_start = 0;
                } else {
                    ext_start = int(mid - _fragcenter);
                }
                ext_end = int(mid + _fragcenter);
            }

            if (_obeySplits == true) {
                bedVector bedBlocks; // vec to store the discrete BED "blocks"
                GetBedBlocks(a, bedBlocks);
                AddBlockedCoverage(bedBlocks);
            }
            else if (_only_5p_end) {
                CHRPOS pos = ( a.strand=="+" ) ? ext_start : ext_end-1;
                AddCoverage(pos,pos);
            }
            else if (_only_3p_end) {
                CHRPOS pos = ( a.strand=="-" ) ? ext_start : ext_end-1;
                AddCoverage(pos,pos);
            }
            else
                AddCoverage(ext_start, ext_end-1);
        }
    }
    _bed->Close();

    // process the results of the last chromosome.
    ReportChromCoverage(_currChromCoverage, _currChromSize,
            _currChromName, _currChromDepthHist);

    // report all empty chromsomes
    PrintEmptyChromosomes();

    // report the overall coverage if asked.
    PrintFinalCoverage();
}


void BedGenomeCoverage::PrintEmptyChromosomes()
{
    // get the list of chromosome names in the genome
    vector<string> chromList = _genome->getChromList();
    vector<string>::const_iterator chromItr = chromList.begin();
    vector<string>::const_iterator chromEnd = chromList.end();
    for (; chromItr != chromEnd; ++chromItr) {
        string chrom = *chromItr;
        if (_visitedChromosomes.find(chrom) == _visitedChromosomes.end()) {
            _currChromName = chrom;
            _currChromSize = _genome->getChromSize(_currChromName);
            std::vector<DEPTH>().swap(_currChromCoverage);
            _currChromCoverage.resize(_currChromSize);
            for (CHRPOS i = 0; i < _currChromSize; ++i)
            {
                _currChromCoverage[i].starts = 0;
                _currChromCoverage[i].ends = 0;
            }
            ReportChromCoverage(_currChromCoverage, _currChromSize,
                    _currChromName, _currChromDepthHist);
        }
    }
}

void BedGenomeCoverage::PrintFinalCoverage()
{
    if (_orderchrom) {
        // get the list of chromosome names in the genome
        vector<string> chromList = _genome->getChromList();

        vector<string>::const_iterator chromItr = chromList.begin();
        vector<string>::const_iterator chromEnd = chromList.end();
        for (; chromItr != chromEnd; ++chromItr) {
            string chrom = *chromItr;
            string nametmp = gettmpfile(chrom);
            if (nametmp != "") {
                std::ifstream __cin(nametmp.c_str(), ios::in);
                if (__cin &&  __cin.peek() != std::ifstream::traits_type::eof()){
                    cout << __cin.rdbuf();
                }
                unlink(nametmp.c_str());
            }
        }
    }
    if (_eachBase == false && _bedGraph == false && _bedGraphAll == false) {
        ReportGenomeCoverage(_currChromDepthHist);
    }
}


void BedGenomeCoverage::CoverageBam(string bamFile) {

    ResetChromCoverage();

    // open the BAM file
    BamReader reader;
    if (!reader.Open(bamFile)) {
        cerr << "Failed to open BAM file " << bamFile << endl;
        exit(1);
    }

    // get header & reference information
    string header = reader.GetHeaderText();
    RefVector refs = reader.GetReferenceData();

    if (_orderchrom) {
        // if need order by genome file, ignore BAM header
        _genome = new GenomeFile(_genomeFile);
    } else {
        // load the BAM header references into a BEDTools "genome file"
        _genome = new GenomeFile(refs);
    }
    // convert each aligned BAM entry to BED
    // and compute coverage on B
    BamAlignment bam;
    while (reader.GetNextAlignment(bam)) {
        // skip if the read is unaligned
        if (bam.IsMapped() == false)
            continue;

        bool _isReverseStrand = bam.IsReverseStrand();

        //changing second mate's strand to opposite
        if( _dUTP && bam.IsPaired() && bam.IsMateMapped() && bam.IsSecondMate())
            _isReverseStrand = !bam.IsReverseStrand();

        // skip if we care about strands and the strand isn't what
        // the user wanted
        if ( (_filterByStrand == true) &&
             ((_requestedStrand == "-") != _isReverseStrand) )
            continue;

        // extract the chrom, start and end from the BAM alignment
        string chrom(refs.at(bam.RefID).RefName);
        CHRPOS start = bam.Position;
        CHRPOS end = bam.GetEndPosition(false, false) - 1;

        // are we on a new chromosome?
        if ( chrom != _currChromName )
            StartNewChrom(chrom);
        if(_pair_chip_) {
            // Skip if not a proper pair
            if (bam.IsPaired() && (!bam.IsProperPair() or !bam.IsMateMapped()) )
                continue;
            // Skip if wrong coordinates
            if( ( (bam.Position<bam.MatePosition) && bam.IsReverseStrand() ) ||
                ( (bam.MatePosition < bam.Position) && bam.IsMateReverseStrand() ) ) {
                    //chemically designed: left on positive strand, right on reverse one
                    continue;
            }

            /*if(_haveSize) {
                if (bam.IsFirstMate() && bam.IsReverseStrand()) { //put fragmentSize in to the middle of pair end_fragment
                    int mid = bam.MatePosition+abs(bam.InsertSize)/2;
                    if(mid<_fragmentSize/2)
                        AddCoverage(0, mid+_fragmentSize/2);
                    else
                        AddCoverage(mid-_fragmentSize/2, mid+_fragmentSize/2);
                }
                else if (bam.IsFirstMate() && bam.IsMateReverseStrand()) { //put fragmentSize in to the middle of pair end_fragment
                    int mid = start+abs(bam.InsertSize)/2;
                    if(mid<_fragmentSize/2)
                        AddCoverage(0, mid+_fragmentSize/2);
                    else
                        AddCoverage(mid-_fragmentSize/2, mid+_fragmentSize/2);
                }
            } else */

            if (bam.IsFirstMate() && bam.IsReverseStrand()) { //prolong to the mate to the left
                AddCoverage(bam.MatePosition, end);
            }
            else if (bam.IsFirstMate() && bam.IsMateReverseStrand()) { //prolong to the mate to the right
                AddCoverage(start, start + abs(bam.InsertSize) - 1);
            }
        } else if (_haveSize) {
            CHRPOS ext_start = start;
            CHRPOS ext_end = end;
            if (_fragmentSize_rev > 0 && bam.IsReverseStrand()) {
                if(ext_end<_fragmentSize_rev) { //sometimes fragmentSize is bigger :(
                    ext_start = 0;
                } else {
                    ext_start = ext_end - _fragmentSize_rev + 1;
                }
                ext_end = ext_end + 1;
            } else if (_fragmentSize > 0 && !bam.IsReverseStrand()) {
                ext_end = ext_start + _fragmentSize;
            }
            if (_fragcenter_rev > -1 && bam.IsReverseStrand()) {
                float mid = float((float(ext_start) + float(ext_end) + 1)/2);
                if (mid<_fragcenter_rev) {
                    ext_start = 0;
                } else {
                    ext_start = int(mid - _fragcenter_rev);
                }
                ext_end = int(mid + _fragcenter_rev);
            } else if (_fragcenter > -1 && !bam.IsReverseStrand()) {
                float mid = float((float(ext_start) + float(ext_end) + 1)/2);
                if (mid<_fragcenter) {
                    ext_start = 0;
                } else {
                    ext_start = int(mid - _fragcenter);
                }
                ext_end = int(mid + _fragcenter);
            }
            AddCoverage(ext_start, ext_end - 1);
        } else
        // add coverage accordingly.
        if (!_only_5p_end && !_only_3p_end) {
            bedVector bedBlocks;
            // we always want to split blocks when a D CIGAR op is found.
            // if the user invokes -split, we want to also split on N ops.
            if (_obeySplits) { // "D" true, "N" true
                GetBamBlocks(bam, refs.at(bam.RefID).RefName, bedBlocks, true, true);
            }
            else { // "D" true, "N" false
                GetBamBlocks(bam, refs.at(bam.RefID).RefName, bedBlocks, true, false);
            }
            AddBlockedCoverage(bedBlocks);
        }
        else if (_only_5p_end) {
            CHRPOS pos = ( !bam.IsReverseStrand() ) ? start : end;
            AddCoverage(pos,pos);
        }
        else if (_only_3p_end) {
            CHRPOS pos = ( bam.IsReverseStrand() ) ? start : end;
            AddCoverage(pos,pos);
        }
    }
    // close the BAM
    reader.Close();

    // process the results of the last chromosome.
    ReportChromCoverage(_currChromCoverage, _currChromSize,
            _currChromName, _currChromDepthHist);

    // report all empty chromsomes
    PrintEmptyChromosomes();

    // report the overall coverage if asked.
    PrintFinalCoverage();
}


void BedGenomeCoverage::ReportChromCoverage(const vector<DEPTH> &chromCov, const CHRPOS &chromSize, const string &chrom, chromHistMap &chromDepthHist) {
    // write to tempory files to reorder
    if (_orderchrom){
        if (_cout){
            (*_cout).flush();
        }
        char tmpname[] = "tmp.genomecov.XXXXXXXX";
        std::string nametmp = mktemp(tmpname);
        ofstream _couttmp(nametmp.c_str(), ios::out);
        if ( !_couttmp ) {
            cerr << "Error: The requested file ("
                << nametmp
                << ") "
                << "could not be opened. "
                << "Exiting!" << endl;
            exit (1);
        } else {
            _couttmp.close();
            _cout = new ofstream(nametmp.c_str(), ios::out);
        }
        _orderchrom_tmp_files[chrom] = nametmp;
    } else {
        _cout = &cout;
        std::string nametmp = "stdout";
    }

    if (_eachBase) {
        int depth = 0; // initialize the depth
        CHRPOS offset = (_eachBaseZeroBased)?0:1;
        for (CHRPOS pos = 0; pos < chromSize; pos++) {

            depth += chromCov[pos].starts;
            // report the depth for this position.
            if (depth>0 || !_eachBaseZeroBased)
                *_cout << chrom << "\t" << pos+offset << "\t" << depth * _scale << '\n';
            depth = depth - chromCov[pos].ends;
        }
    }
    else if (_bedGraph == true || _bedGraphAll == true) {
        ReportChromCoverageBedGraph(chromCov, chromSize, chrom);
    }
    else {

        int depth = 0; // initialize the depth

        for (CHRPOS pos = 0; pos < chromSize; pos++) {

            depth += chromCov[pos].starts;

            // add the depth at this position to the depth histogram
            // for this chromosome. if the depth is greater than the
            // maximum bin requested, then readjust the depth to be the max
            if (depth >= _max) {
                chromDepthHist[chrom][_max]++;
            }
            else {
                chromDepthHist[chrom][depth]++;
            }
            depth = depth - chromCov[pos].ends;
        }
        // report the histogram for each chromosome
        histMap::const_iterator depthIt = chromDepthHist[chrom].begin();
        histMap::const_iterator depthEnd = chromDepthHist[chrom].end();
        for (; depthIt != depthEnd; ++depthIt) {
            int depth = depthIt->first;
            unsigned int numBasesAtDepth = depthIt->second;
            cout << chrom << "\t" << depth << "\t" << numBasesAtDepth << "\t"
                << chromSize << "\t" << (float) ((float)numBasesAtDepth / (float)chromSize) << endl;
        }
    }
    if (_orderchrom){
        if (_cout){
            (*_cout).flush();
        }
    }
}



void BedGenomeCoverage::ReportGenomeCoverage(chromHistMap &chromDepthHist) {

    // get the list of chromosome names in the genome
    vector<string> chromList = _genome->getChromList();

    CHRPOS genomeSize = 0;
    vector<string>::const_iterator chromItr = chromList.begin();
    vector<string>::const_iterator chromEnd = chromList.end();
    for (; chromItr != chromEnd; ++chromItr) {
        string chrom = *chromItr;
        genomeSize += _genome->getChromSize(chrom);
        // if there were no reads for a give chromosome, then
        // add the length of the chrom to the 0 bin.
        if ( chromDepthHist.find(chrom) == chromDepthHist.end() ) {
            chromDepthHist[chrom][0] += _genome->getChromSize(chrom);
        }
    }

    histMap genomeHist; // depth histogram for the entire genome

    // loop through each chromosome and add the depth and number of bases at each depth
    // to the aggregate histogram for the entire genome
    for (chromHistMap::iterator chromIt = chromDepthHist.begin(); chromIt != chromDepthHist.end(); ++chromIt) {
        string chrom = chromIt->first;
        for (histMap::iterator depthIt = chromDepthHist[chrom].begin(); depthIt != chromDepthHist[chrom].end(); ++depthIt) {
            int depth = depthIt->first;
            CHRPOS numBasesAtDepth = depthIt->second;
            genomeHist[depth] += numBasesAtDepth;
        }
    }

    // loop through the depths for the entire genome
    // and report the number and fraction of bases in
    // the entire genome that are at said depth.
    for (histMap::iterator genomeDepthIt = genomeHist.begin(); genomeDepthIt != genomeHist.end(); ++genomeDepthIt) {
        int depth = genomeDepthIt->first;
        CHRPOS numBasesAtDepth = genomeDepthIt->second;

        cout << "genome" << "\t" << depth << "\t" << numBasesAtDepth << "\t"
            << genomeSize << "\t" << (float) ((float)numBasesAtDepth / (float)genomeSize) << endl;
    }
}


void BedGenomeCoverage::ReportChromCoverageBedGraph(const vector<DEPTH> &chromCov, const CHRPOS &chromSize, const string &chrom) {

    int depth = 0; // initialize the depth
    CHRPOS lastStart = -1;
    int lastDepth = -1;

    for (CHRPOS pos = 0; pos < chromSize; pos++) {
        depth += chromCov[pos].starts;

        if (depth != lastDepth) {
            // Coverage depth has changed, print the last interval coverage (if any)
            // Print if:
            // (1) depth>0 (the default running mode),
            // (2) depth==0 and the user requested to print zero covered regions (_bedGraphAll)
            if ( (lastDepth != -1) && (lastDepth > 0 || _bedGraphAll) ) {

                if (lastDepth >= _max) {
                    lastDepth = _max;
                }
                *_cout << chrom << "\t" << lastStart << "\t" << pos << "\t" << lastDepth * _scale << '\n';
            }
            //Set current position as the new interval start + depth
            lastDepth = depth;
            lastStart = pos;
        }
        // Default: the depth has not changed, so we will not print anything.
        // Proceed until the depth changes.
        // Update depth
        depth = depth - chromCov[pos].ends;
    }
    //Print information about the last position
    if ( (lastDepth != -1) && (lastDepth > 0 || _bedGraphAll) ) {
        *_cout << chrom << "\t" << lastStart << "\t" << chromSize << "\t" << lastDepth * _scale << endl;
    }
}

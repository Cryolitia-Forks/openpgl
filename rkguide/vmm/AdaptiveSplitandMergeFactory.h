// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../rkguide.h"
#include "VMM.h"
#include "../data/DirectionalSampleData.h"

#include "WeightedEMVMMFactory.h"
#include "WeightedEMParallaxAwareVMMFactory.h"
#include "VMMChiSquareComponentSplitter.h"
#include "VMMChiSquareComponentMerger.h"

#include <fstream>
#include <iostream>

namespace rkguide
{

template<class TVMMDistribution>
struct AdaptiveSplitAndMergeFactory
{

public:
    typedef TVMMDistribution VMM;

    //typedef WeightedEMVonMisesFisherFactory<VMM> WeightedEMFactory;
    typedef WeightedEMParallaxAwareVonMisesFisherFactory<VMM> WeightedEMFactory;
    typedef VonMisesFisherChiSquareComponentSplitter<WeightedEMFactory> Splitter;
    typedef VonMisesFisherChiSquareComponentMerger<WeightedEMFactory> Merger;


    struct ASMConfiguration
    {
        typename WeightedEMFactory::Configuration weightedEMCfg;

        float splittingThreshold { 0.75 };
        float mergingThreshold { 0.00625 };

        bool useSplitAndMerge {true};

        bool partialReFit { false };
        int maxSplitItr { 1 };

        int minSamplesForSplitting { 0 };
        int minSamplesForPartialRefitting { 0 };
        int minSamplesForMerging { 0 };

        void serialize(std::ostream& stream) const;

        void deserialize(std::istream& stream);

        std::string toString() const;
    };


    struct ASMStatistics
    {
        typename WeightedEMFactory::SufficientStatisitcs sufficientStatistics;
        typename Splitter::ComponentSplitStatistics splittingStatistics;

        //size_t numComponents {0};

        size_t numSamplesAfterLastSplit {0};
        size_t numSamplesAfterLastMerge {0};

        ASMStatistics() = default;
        ASMStatistics(const ASMStatistics &a); // = delete;

        void clear(const size_t &_numComponents);
        void clearAll();

        void decay(const float &alpha);

        void serialize(std::ostream& stream) const;

        void deserialize(std::istream& stream);

        bool isValid() const;

        inline size_t getNumComponents() const
        {
            RKGUIDE_ASSERT(sufficientStatistics.getNumComponents() == splittingStatistics.getNumComponents());
            return sufficientStatistics.getNumComponents();
        }

        std::string toString() const;

    };

    struct ASMFittingStatistics
    {
        size_t numSamples {0};
        size_t numSplits {0};
        size_t numMerges {0};

        size_t numComponents {0};

        size_t numUpdateWEMIterations {0};
        size_t numPartialUpdateWEMIterations {0};

        std::string toString() const;
    };

    void fit(VMM &vmm, size_t numComponents, ASMStatistics &stats, const DirectionalSampleData* samples, const size_t numSamples, const ASMConfiguration &cfg, ASMFittingStatistics &fitStats) const;

    void update(VMM &vmm, ASMStatistics &stats, const DirectionalSampleData* samples, const size_t numSamples, const ASMConfiguration &cfg, ASMFittingStatistics &fitStats) const;

    std::string toString() const{
        std::ostringstream oss;
        WeightedEMFactory vmmFactory;
        oss << "AdaptiveSplitAndMergeFactory[\n"
            << "  VMMFactory: " << vmmFactory.toString() << '\n'
            << ']';

        return oss.str();
    }

};

template<class TVMMDistribution>
AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::ASMStatistics(const AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics &a)
{
    this->sufficientStatistics = a.sufficientStatistics;
    this->splittingStatistics = a.splittingStatistics;

    this->numSamplesAfterLastSplit = a.numSamplesAfterLastSplit;
    this->numSamplesAfterLastMerge = a.numSamplesAfterLastMerge;
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::serialize(std::ostream& stream) const
{
    sufficientStatistics.serialize(stream);
    splittingStatistics.serialize(stream);

    //stream.write(reinterpret_cast<const char*>(&numComponents), sizeof(size_t));
    stream.write(reinterpret_cast<const char*>(&numSamplesAfterLastSplit), sizeof(size_t));
    stream.write(reinterpret_cast<const char*>(&numSamplesAfterLastMerge), sizeof(size_t));
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::deserialize(std::istream& stream)
{
    sufficientStatistics.deserialize(stream);
    splittingStatistics.deserialize(stream);

    //stream.read(reinterpret_cast<char*>(&numComponents), sizeof(size_t));
    stream.read(reinterpret_cast<char*>(&numSamplesAfterLastSplit), sizeof(size_t));
    stream.read(reinterpret_cast<char*>(&numSamplesAfterLastMerge), sizeof(size_t));
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::decay(const float &alpha)
{
    sufficientStatistics.decay(alpha);
    splittingStatistics.decay(alpha);
}


template<class TVMMDistribution>
bool AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::isValid() const
{
    bool valid = true;
    valid &= sufficientStatistics.isValid();
    RKGUIDE_ASSERT(valid);
    valid &= splittingStatistics.isValid();
    RKGUIDE_ASSERT(valid);
    valid &= isvalid(numSamplesAfterLastSplit);
    RKGUIDE_ASSERT(valid);
    valid &= isvalid(numSamplesAfterLastMerge);
    RKGUIDE_ASSERT(valid);

    return valid;
}

template<class TVMMDistribution>
std::string AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::toString() const
{
    std::stringstream ss;
    ss << "ASMStatistics:" << std::endl;
    ss << "\tsufficientStatistics:" << sufficientStatistics.toString() << std::endl;
    ss << "\tsplittingStatistics:" << splittingStatistics.toString() << std::endl;
    ss << "\tnumSamplesAfterLastSplit = " << numSamplesAfterLastSplit << std::endl;
    ss << "\tnumSamplesAfterLastMerge = " << numSamplesAfterLastMerge << std::endl;
    return ss.str();
}

template<class TVMMDistribution>
std::string AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMFittingStatistics::toString() const
{
    std::stringstream ss;
    ss << "ASMFittingStatistics:" << std::endl;
    ss << "\tnumSamples:" << numSamples << std::endl;
    ss << "\tnumSplits:" << numSplits << std::endl;
    ss << "\tnumMerges:" << numMerges << std::endl;
    ss << "\tnumComponents:" << numComponents << std::endl;
    ss << "\tnumUpdateWEMIterations:" << numUpdateWEMIterations << std::endl;
    ss << "\tnumPartialUpdateWEMIterations:" << numPartialUpdateWEMIterations << std::endl;
    return ss.str();
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::clear(const size_t &_numComponents)
{
    sufficientStatistics.clear(_numComponents);
    splittingStatistics.clear(_numComponents);

    //numComponents = _numComponents;
    numSamplesAfterLastSplit = 0;
    numSamplesAfterLastMerge = 0;
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMStatistics::clearAll()
{
    clear(VMM::MaxComponents);
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMConfiguration::serialize(std::ostream& stream) const
{
    weightedEMCfg.serialize(stream);

    stream.write(reinterpret_cast<const char*>(&splittingThreshold), sizeof(float));
    stream.write(reinterpret_cast<const char*>(&mergingThreshold), sizeof(float));

    stream.write(reinterpret_cast<const char*>(&partialReFit), sizeof(bool));
    stream.write(reinterpret_cast<const char*>(&maxSplitItr), sizeof(int));

    stream.write(reinterpret_cast<const char*>(&minSamplesForSplitting), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&minSamplesForMerging), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&minSamplesForPartialRefitting), sizeof(int));
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMConfiguration::deserialize(std::istream& stream)
{
    weightedEMCfg.deserialize(stream);

    stream.read(reinterpret_cast<char*>(&splittingThreshold), sizeof(float));
    stream.read(reinterpret_cast<char*>(&mergingThreshold), sizeof(float));

    stream.read(reinterpret_cast<char*>(&partialReFit), sizeof(bool));
    stream.read(reinterpret_cast<char*>(&maxSplitItr), sizeof(int));

    stream.read(reinterpret_cast<char*>(&minSamplesForSplitting), sizeof(int));
    stream.read(reinterpret_cast<char*>(&minSamplesForMerging), sizeof(int));
    stream.read(reinterpret_cast<char*>(&minSamplesForPartialRefitting), sizeof(int));
}

template<class TVMMDistribution>
std::string AdaptiveSplitAndMergeFactory<TVMMDistribution>::ASMConfiguration::toString() const
{
    std::stringstream ss;
    ss << "ASMConfiguration:" << std::endl;
    ss << "\tweightedEMCfg = " << weightedEMCfg.toString() << std::endl;
    ss << "\tsplittingThreshold = " << splittingThreshold << std::endl;
    ss << "\tmergingThreshold = " << mergingThreshold << std::endl;
    ss << "\tuseSplitAndMerge = " << useSplitAndMerge << std::endl;
    ss << "\tpartialReFit = " << partialReFit << std::endl;
    ss << "\tmaxSplitItr = " << maxSplitItr << std::endl;
    ss << "\tminSamplesForSplitting = " << minSamplesForSplitting << std::endl;
    ss << "\tminSamplesForPartialRefitting = " << minSamplesForPartialRefitting << std::endl;
    ss << "\tminSamplesForMerging = " << minSamplesForMerging << std::endl;
    return ss.str();
}

template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::fit(VMM &vmm, size_t numComponents, ASMStatistics &stats, const DirectionalSampleData* samples, const size_t numSamples, const ASMConfiguration &cfg, ASMFittingStatistics &fitStats) const
{
    // intial fit
    WeightedEMFactory factory = WeightedEMFactory();
    typename WeightedEMFactory::FittingStatistics wemFitStats;
    factory.fitMixture(vmm, numComponents, stats.sufficientStatistics, samples, numSamples, cfg.weightedEMCfg, wemFitStats);
    factory.initComponentDistances(vmm, stats.sufficientStatistics, samples, numSamples);
    RKGUIDE_ASSERT(vmm.isValid());
    RKGUIDE_ASSERT(vmm.getNumComponents() == stats.sufficientStatistics.getNumComponents());
    RKGUIDE_ASSERT(stats.sufficientStatistics.isValid());
/* */

    if (cfg.useSplitAndMerge)
    {

        // calculate the estimate of the integral of the function (e.g. radiance or importance) fitted by the VMM
        float mcEstimate = stats.sufficientStatistics.getSumWeights() / stats.sufficientStatistics.getNumSamples();

        // split the fitted components of the inital fit to match
        // the observed samples
    #ifdef RKGUIDE_SHOW_PRINT_OUTS
        std::cout << stats.sufficientStatistics.toString() << std::endl;
    #endif
        Splitter splitter = Splitter();
        splitter.PerformRecursiveSplitting(vmm, stats.sufficientStatistics, cfg.splittingThreshold, mcEstimate, samples, numSamples, cfg.weightedEMCfg);

        splitter.CalculateSplitStatistics(vmm, stats.splittingStatistics, mcEstimate, samples, numSamples);

        //std::cout << "fit: numComponents: " << vmm._numComponents << std::endl;

        //RKGUIDE_ASSERT(vmm._numComponents == stats.sufficientStatistics.numComponents);
        RKGUIDE_ASSERT(vmm.getNumComponents() == stats.getNumComponents());
        RKGUIDE_ASSERT(vmm.isValid());

        Merger merger = Merger();
        merger.PerformMerging(vmm, cfg.mergingThreshold, stats.sufficientStatistics, stats.splittingStatistics);
        RKGUIDE_ASSERT(vmm.isValid());
    /* */
        //std::cout << "fit: numComponents: " << vmm._numComponents << std::endl;
        //stats.splittingStatistics.clear(vmm._numComponents);
    }
    stats.numSamplesAfterLastSplit = 0.0f;
    stats.numSamplesAfterLastMerge = 0.0f;

    factory.initComponentDistances(vmm, stats.sufficientStatistics, samples, numSamples);
    RKGUIDE_ASSERT(stats.sufficientStatistics.isValid());
    RKGUIDE_ASSERT(vmm.isValid());
}



template<class TVMMDistribution>
void AdaptiveSplitAndMergeFactory<TVMMDistribution>::update(VMM &vmm, ASMStatistics &stats, const DirectionalSampleData* samples, const size_t numSamples, const ASMConfiguration &cfg, ASMFittingStatistics &fitStats) const
{
    RKGUIDE_ASSERT(vmm.isValid());
    RKGUIDE_ASSERT(vmm.getNumComponents() == stats.getNumComponents());
    RKGUIDE_ASSERT(stats.isValid());

    // first update the mixture
    WeightedEMFactory factory = WeightedEMFactory();
    typename WeightedEMFactory::FittingStatistics wemFitStats;
    //stats.sufficientStatistics.clear(vmm._numComponents);
    factory.updateMixture(vmm, stats.sufficientStatistics, samples, numSamples, cfg.weightedEMCfg, wemFitStats);

    RKGUIDE_ASSERT(vmm.isValid());
    RKGUIDE_ASSERT(stats.sufficientStatistics.isValid());

    if (cfg.useSplitAndMerge)
    {

        float mcEstimate = stats.sufficientStatistics.getSumWeights() / stats.sufficientStatistics.getNumSamples();

        fitStats.numSamples = numSamples;
        fitStats.numUpdateWEMIterations = wemFitStats.numIterations;

        stats.numSamplesAfterLastSplit += numSamples;
        stats.numSamplesAfterLastMerge += numSamples;

        Splitter splitter = Splitter();
        //stats.splittingStatistics.clear(vmm._numComponents);
        //RKGUIDE_ASSERT(stats.splittingStatistics.isValid());
        splitter.UpdateSplitStatistics(vmm, stats.splittingStatistics, mcEstimate, samples, numSamples);
        RKGUIDE_ASSERT(stats.splittingStatistics.isValid());
        //splitter.CalculateSplitStatistics(vmm, stats.splittingStatistics, mcEstimate, samples, numSamples);


    //    std::cout << "numSamplesAfterLastSplit: " << stats.numSamplesAfterLastSplit << "\t minSamplesForSplitting: " << cfg.minSamplesForSplitting << std::endl;

        if (stats.numSamplesAfterLastSplit >= cfg.minSamplesForSplitting)
        {
            typename WeightedEMFactory::PartialFittingMask mask;
            mask.resetToFalse();

            std::vector<typename Splitter::SplitCandidate> splitComps = stats.splittingStatistics.getSplitCandidates();
            int totalSplitCount = 0;
            //const size_t numComp = vmm._numComponents;
            for (size_t k = 0; k < splitComps.size(); k++)
            {
                if (splitComps[k].chiSquareEst > cfg.splittingThreshold && vmm._numComponents  < VMM::MaxComponents)
                {
                    bool splitSucess = splitter.SplitComponent(vmm, stats.splittingStatistics, stats.sufficientStatistics, splitComps[k].componentIndex);
                    mask.setToTrue(splitComps[k].componentIndex);
                    mask.setToTrue(vmm._numComponents-1);
                    //std::cout << "split[" << totalSplitCount << "]: " << "\tidx0: " << splitComps[k].componentIndex << "\tidx1: " << vmm._numComponents-1 << std::endl;
                    totalSplitCount++;

                }
            }

            RKGUIDE_ASSERT(vmm.isValid());
            RKGUIDE_ASSERT(vmm.getNumComponents() == stats.getNumComponents());
            RKGUIDE_ASSERT(stats.isValid());

            if (totalSplitCount > 0 &&  cfg.partialReFit && numSamples >= cfg.minSamplesForPartialRefitting)
            {
                typename WeightedEMFactory::SufficientStatisitcs tempSuffStatistics = stats.sufficientStatistics;
                tempSuffStatistics.clear(vmm._numComponents);
                factory.partialUpdateMixture(vmm, mask, tempSuffStatistics, samples, numSamples, cfg.weightedEMCfg, wemFitStats);
                stats.sufficientStatistics.setNumComponents(vmm._numComponents);
                stats.sufficientStatistics.maskedReplace(mask, tempSuffStatistics);

                fitStats.numPartialUpdateWEMIterations = wemFitStats.numIterations;
                RKGUIDE_ASSERT(vmm.isValid());
                RKGUIDE_ASSERT(vmm.getNumComponents() == stats.getNumComponents());
                RKGUIDE_ASSERT(stats.isValid());
            }

            fitStats.numSplits = totalSplitCount;
            stats.numSamplesAfterLastSplit = 0.0f;

#ifdef RKGUIDE_SHOW_PRINT_OUTS
            std::cout << "update: totalSplitCount = " << totalSplitCount << "\t splitThreshold: " << cfg.splittingThreshold<< std::endl;
#endif
        }

        RKGUIDE_ASSERT(vmm.isValid());
        RKGUIDE_ASSERT(vmm.getNumComponents() == stats.getNumComponents());
        RKGUIDE_ASSERT(stats.isValid());

        if (stats.numSamplesAfterLastMerge >= cfg.minSamplesForMerging)
        {
            Merger merger = Merger();
            size_t numMerges = merger.PerformMerging(vmm, cfg.mergingThreshold, stats.sufficientStatistics, stats.splittingStatistics);
            fitStats.numMerges = numMerges;
            stats.numSamplesAfterLastMerge = 0.0f;

            RKGUIDE_ASSERT(vmm.isValid());
            RKGUIDE_ASSERT(vmm.getNumComponents() == stats.getNumComponents());
            RKGUIDE_ASSERT(stats.isValid());
        }

        fitStats.numComponents = vmm._numComponents;
    }

    factory.updateComponentDistances(vmm, stats.sufficientStatistics, samples, numSamples);

    RKGUIDE_ASSERT(vmm.getNumComponents() == stats.sufficientStatistics.getNumComponents());
    RKGUIDE_ASSERT(vmm.isValid());
    RKGUIDE_ASSERT(stats.sufficientStatistics.isValid());
}


}


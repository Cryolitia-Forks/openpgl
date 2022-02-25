// Copyright 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../openpgl.h"
#include "Region.h"
#include "Field.h"

namespace openpgl
{
namespace cpp
{

/**
 * @brief The Sampling distriubtion used for guidiging directional sampling decisions on surfaces.
 * 
 * The guided sampling distribution can be proportional to the incomming radiance or its product
 * with components of a BSDF model (e.g., cosine term). The class supports function for sampling and
 * PDF evalautions. 
 * 
 */
struct SurfaceSamplingDistribution
{
    /**
     * @brief Constructs new instance of a SurfaceSamplingDistribution.
     * 
     * Reserves the memory need to store the guiding distriubtion.
     * Since the type/representation of distribution depends on the guiding field
     * a pointer to the @ref Field has to be provided. After construction
     * the SurfaceSamplingDistribution still need to be initialized using the @ref Init function.
     * 
     * @param field Pointer to the -guiding- Field.
     */
    SurfaceSamplingDistribution(const Field* field);

    ~SurfaceSamplingDistribution();

    SurfaceSamplingDistribution(const SurfaceSamplingDistribution&) = delete;

    /**
     * @brief Intitializes the guiding distibution for a given position in the scene.
     * 
     * This function queries the guiding field for a surface guiding distribution for
     * given position in the scene and initializes the SurfaceSamplingDistribution
     * to this distriubtion. The resulting distribution is usually proportional to the local
     * incident radiance distriubtion at the query position. The SurfaceSamplingDistribution
     * can further being imporoved by applying products with BSDF components (e.g., cosine). 
     * 
     * @param field The guiding field of the scene.
     * @param pos The position the guiding distribution is queried for.
     * @param sample1D A random number used of a stoachastic look-up is used.
     * @param useParallaxCompensation If parallax compensation sould be applied or not. @deprecated
     * @return true 
     * @return false 
     */
    bool Init(const Field* field, const pgl_point3f& pos, float& sample1D, const bool useParallaxCompensation = true);

    /**
     * @brief Clears/resets the internal repesentation of the guiding distribution. 
     * 
     */
    void Clear();

    /**
     * @brief Importance samples a new direction based on the guiding distriubtion.
     * 
     * @param sample2D A 2D random variable
     * @return pgl_vec3f The sampled direction
     */
    pgl_vec3f Sample(const pgl_point2f& sample2D)const;

    /**
     * @brief Returns the sampling PDF for a given direction when is sampled
     * according to the guiding distribution.
     * 
     * @param direction 
     * @return float The PDF for sampling @ref direction
     */
    float PDF(const pgl_vec3f& direction) const;


    /**
     * @brief Combined importance sampling and PDF calculation.
     * Can be more efficient to use for some distributions (e.g. DirectionQuadtree)
     * 
     * @param sample2D A 2D random variable
     * @param direction Importance sampled direction
     * @return float The PDF for sampling @ref direction
     */
    float SamplePDF(const pgl_point2f& sample2D, pgl_vec3f& direction) const;

    /**
     * @brief Returns if the used representation supports for including the cosine 
     * product (e.g, for diffuse surfaces) into the guiding distriubtion. 
     * 
     * @return true 
     * @return false 
     */
    bool SupportsApplyCosineProduct() const;

    /**
     * @brief Applies the product with the cosine to the guiding distriubtion.
     *  
     * @param normal The surface normal at the curren sampling position.
     */
    void ApplyCosineProduct(const pgl_vec3f& normal);

    ///////////////////////////////////////
    /// Future plans
    ///////////////////////////////////////
    
    /**
     * @brief 
     * 
     * @param normal 
     * @param opaque 
     * @param transmission 
     */
    //void ApplyCosineProduct(const pgl_vec3f& normal, const bool opaque, const pgl_vec3f transmission);
    

    /**
     * @brief Validates the current guiding distribution.
     * The guiding distriubtion can be invalid if it was not
     * initialized before or due to (numerical) porblems during the fitting process.
     * 
     * Note: Due to the overhead of this function, it should only be called during debugging.
     * 
     * @return true 
     * @return false 
     */
    bool Validate() const;


    /**
     * @brief @deprecated
     * 
     * @return Region 
     */
    Region GetRegion() const;

    private:
        PGLSurfaceSamplingDistribution m_surfaceSamplingDistributionHandle{nullptr};
};

////////////////////////////////////////////////////////////
/// Implementation
////////////////////////////////////////////////////////////

/*
SurfaceSamplingDistribution::SurfaceSamplingDistribution()
{
    m_surfaceSamplingDistributionHandle = pglNewSurfaceSamplingDistribution();
}
*/
OPENPGL_INLINE SurfaceSamplingDistribution::SurfaceSamplingDistribution(const Field* field)
{
    m_surfaceSamplingDistributionHandle = pglFieldNewSurfaceSamplingDistribution(field->m_fieldHandle);
}

OPENPGL_INLINE SurfaceSamplingDistribution::~SurfaceSamplingDistribution()
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    if(m_surfaceSamplingDistributionHandle)
        pglReleaseSurfaceSamplingDistribution(m_surfaceSamplingDistributionHandle);
    m_surfaceSamplingDistributionHandle = nullptr;
}


OPENPGL_INLINE pgl_vec3f SurfaceSamplingDistribution::Sample(const pgl_point2f& sample2D)const
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    return pglSurfaceSamplingDistributionSample(m_surfaceSamplingDistributionHandle, sample2D);
}

OPENPGL_INLINE float SurfaceSamplingDistribution::PDF(const pgl_vec3f& direction) const
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    return pglSurfaceSamplingDistributionPDF(m_surfaceSamplingDistributionHandle, direction);
}

OPENPGL_INLINE float SurfaceSamplingDistribution::SamplePDF(const pgl_point2f& sample2D, pgl_vec3f& direction) const
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    return pglSurfaceSamplingDistributionSamplePDF(m_surfaceSamplingDistributionHandle, sample2D, direction);    
}

OPENPGL_INLINE bool SurfaceSamplingDistribution::Validate() const
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    return pglSurfaceSamplingDistributionValidate(m_surfaceSamplingDistributionHandle);
}

OPENPGL_INLINE void SurfaceSamplingDistribution::Clear()
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    return pglSurfaceSamplingDistributionClear(m_surfaceSamplingDistributionHandle);
}
/*
void SurfaceSamplingDistribution::Init(const Region& region, const pgl_point3f& pos, const bool useParallaxCompensation)
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    pglSurfaceSamplingDistributionInit(m_surfaceSamplingDistributionHandle, region.m_regionHandle, pos, useParallaxCompensation);
}
*/
OPENPGL_INLINE bool SurfaceSamplingDistribution::Init(const Field* field, const pgl_point3f& pos, float& sample1D, const bool useParallaxCompensation)
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    OPENPGL_ASSERT(field->m_fieldHandle);
    return pglFieldInitSurfaceSamplingDistriubtion(field->m_fieldHandle, m_surfaceSamplingDistributionHandle, pos, &sample1D, useParallaxCompensation);
}

OPENPGL_INLINE void SurfaceSamplingDistribution::ApplyCosineProduct(const pgl_vec3f& normal)
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    pglSurfaceSamplingDistributionApplyCosineProduct(m_surfaceSamplingDistributionHandle, normal);
}

OPENPGL_INLINE bool SurfaceSamplingDistribution::SupportsApplyCosineProduct() const
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    pglSurfaceSamplingDistributionSupportsApplyCosineProduct(m_surfaceSamplingDistributionHandle);
}

OPENPGL_INLINE Region SurfaceSamplingDistribution::GetRegion() const
{
    OPENPGL_ASSERT(m_surfaceSamplingDistributionHandle);
    //OPENPGL_ASSERT(sampler);
    //OPENPGL_ASSERT(&sampler->m_samplerHandle);
    PGLRegion regionHandle = pglSurfaceSamplingGetRegion(m_surfaceSamplingDistributionHandle);
    return Region(regionHandle);
}

}
}
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkScalarToRGBPixelFunctor.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkVectorGradientAnisotropicDiffusionImageFilter.h"
#include "itkWatershedImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkScalarToRGBColormapImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"

// Run with:
// ./SegmentWithWatershedImageFilter inputImageFile outputImageFile threshold level
// e.g.
// ./SegmentWithWatershedImageFilter BrainProtonDensitySlice.png OutBrainWatershed.png 0.005 .5
// (A rule of thumb is to set the Threshold to be about 1 / 100 of the Level.)

int
main(int argc, char * argv[])
{
    if (argc < 5)
    {
        std::cerr << "Missing parameters." << std::endl;
        std::cerr << "Usage: " << argv[0] << " inputImageFile outputImageFile threshold level" << std::endl;
        return EXIT_FAILURE;
    }

    constexpr unsigned int Dimension = 3;

    using InputImageType = itk::Image<unsigned char, Dimension>;
    using FloatImageType = itk::Image<float, Dimension>;
    using RGBPixelType = itk::RGBPixel<unsigned char>;
    using RGBImageType = itk::Image<RGBPixelType, Dimension>;
    using LabeledImageType = itk::Image<itk::IdentifierType, Dimension>;

    using FileReaderType = itk::ImageFileReader<InputImageType>;
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(argv[1]);

    using GradientMagnitudeImageFilterType = itk::GradientMagnitudeImageFilter<InputImageType, FloatImageType>;
    GradientMagnitudeImageFilterType::Pointer gradientMagnitudeImageFilter = GradientMagnitudeImageFilterType::New();

    gradientMagnitudeImageFilter->SetInput(reader->GetOutput());
    gradientMagnitudeImageFilter->Update();

    using WatershedFilterType = itk::WatershedImageFilter<FloatImageType>;
    WatershedFilterType::Pointer watershed = WatershedFilterType::New();

    float threshold = std::stod(argv[3]);
    float level = std::stod(argv[4]);

    watershed->SetThreshold(threshold);
    watershed->SetLevel(level);
    auto gradientOutput = gradientMagnitudeImageFilter->GetOutput();
    watershed->SetInput(gradientOutput);
    watershed->Update();

    using RGBFilterType = itk::ScalarToRGBColormapImageFilter<LabeledImageType, RGBImageType>;
    RGBFilterType::Pointer colormapImageFilter = RGBFilterType::New();
    colormapImageFilter->SetColormap(itk::ScalarToRGBColormapImageFilterEnums::RGBColormapFilter::Jet);
    colormapImageFilter->SetInput(watershed->GetOutput());
    colormapImageFilter->Update();

    using FileWriterType = itk::ImageFileWriter<RGBImageType>;
    FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(colormapImageFilter->GetOutput());
    writer->Update();


    return EXIT_SUCCESS;
}
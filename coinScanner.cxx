#include "itkImage.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkBinaryCrossStructuringElement.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryImageToLabelMapFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include <iostream>
#include <stdio.h>

// COIN LABELS
#define MOEDA_1_REAL        0
#define MOEDA_50_CENT       1
#define MOEDA_25_CENT       2
#define MOEDA_10_CENT       3
#define MOEDA_10_CENT_GOLD  4
#define MOEDA_5_CENT        5
#define MOEDA_5_CENT_BRONZE 6

// COIN PIXEL AMOUNTS
#define MOEDA_1_REAL_PIXEL          117000
#define MOEDA_50_CENT_PIXEL         83000
#define MOEDA_25_CENT_PIXEL         100000
#define MOEDA_10_CENT_PIXEL         0
#define MOEDA_10_CENT_GOLD_PIXEL    63117
#define MOEDA_5_CENT_PIXEL          0
#define MOEDA_5_CENT_BRONZE_PIXEL   75000

// ERROR MARGIN
#define MARGEM_ERRO 0.075

// ITK Definitions
typedef itk::Image<unsigned char, 2>  ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>  BinaryThresholdImageFilterType;

// Comparison using an error margin
bool compare(long int size, long int compareValue){ 
    if(size < (compareValue + compareValue*MARGEM_ERRO) && size > (compareValue - compareValue*MARGEM_ERRO)) {
        return true;
    } else {
        return false;
    }
}
  
// Compare a given pixel amount with the defined pixel amounts for each coin
char* findCoinType(long int size) {
    if(compare(size, MOEDA_1_REAL_PIXEL)) {
        return (char *) "1 REAL";
    }
    if(compare(size, MOEDA_50_CENT_PIXEL)) {
        return (char *) "50 CENTAVOS";
    }
    if(compare(size, MOEDA_25_CENT_PIXEL)) {
        return (char *) "25 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_PIXEL)) {
        return (char *) "10 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_GOLD_PIXEL)) {
        return (char *) "10 CENTAVOS GOLD";
    }
    if(compare(size, MOEDA_5_CENT_PIXEL)) {
        return (char *) "5 CENTAVOS";
    }
    if(compare(size, MOEDA_5_CENT_BRONZE_PIXEL)) {
        return (char *) "5 CENTAVOS BRONZE";
    }
    return (char *) "INDEFINIDO";
}

int main(int argc, char *argv[]){

    // Check arguments
    if(argc < 2) {
        printf("Usage: %s [FILE PATH]\n", argv[0]); 
        return 1;
    }
    
    // Declaration of the main variables
    ImageType::Pointer image;
    ImageType::Pointer image1;
    
    // Read input file
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();
 
    // Use a threshold filter to create a binary image
    int lowerThreshold = 10;
    int upperThreshold = 100;
    unsigned int radius = 30;
    BinaryThresholdImageFilterType::Pointer thresholdFilter  = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(reader->GetOutput());
    thresholdFilter->SetLowerThreshold(lowerThreshold);
    thresholdFilter->SetUpperThreshold(upperThreshold);
    thresholdFilter->SetInsideValue(255);
    thresholdFilter->SetOutsideValue(0);

    // Send threshold output to the image
    image = thresholdFilter->GetOutput();
  
    // Apply a binary morphological closing filter to remove noise
    std::cout << "Radius: " << radius << std::endl;
    typedef itk::BinaryCrossStructuringElement<ImageType::PixelType, ImageType::ImageDimension> StructuringElementType;
    StructuringElementType structuringElement;
    structuringElement.SetRadius(radius);
    structuringElement.CreateStructuringElement();

    typedef itk::BinaryMorphologicalClosingImageFilter <ImageType, ImageType, StructuringElementType> BinaryMorphologicalClosingImageFilterType;
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter  = BinaryMorphologicalClosingImageFilterType::New();
    closingFilter->SetInput(image);
    closingFilter->SetKernel(structuringElement);
    closingFilter->Update();
    
    // Apply a reverse filter
    typedef itk::InvertIntensityImageFilter <ImageType>
    InvertIntensityImageFilterType;

    InvertIntensityImageFilterType::Pointer invertIntensityFilter
    = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(closingFilter->GetOutput());
    invertIntensityFilter->SetMaximum(255);

    // Send the morphological output to the image1
    image1 = invertIntensityFilter->GetOutput();

    // Apply an imagetoLabelMap filter to separate objects
    typedef itk::BinaryImageToLabelMapFilter<ImageType> BinaryImageToLabelMapFilterType;
    BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
    binaryImageToLabelMapFilter->SetInput(image1);
    binaryImageToLabelMapFilter->Update();

    // The output of this filter is an itk::LabelMap, which contains itk::LabelObject's
    std::cout << "Numero de objetos: " << binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << std::endl;

    // Loop over each region
    for(unsigned int i = 0; i < binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
        // Get the ith region
        BinaryImageToLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToLabelMapFilter->GetOutput()->GetNthLabelObject(i);

        // Output the number of pixels composing the region
        labelObject->Optimize();
        printf("Objeto %10d - Tamanho: %20ld - Tipo: %20s\n", i+1, labelObject->Size(), findCoinType(labelObject->Size()));
    }

    return EXIT_SUCCESS;
}   
  
  


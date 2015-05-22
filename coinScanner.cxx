/* INCLUDES */

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

/* COIN LABELS */
#define MOEDA_1_REAL        0
#define MOEDA_50_CENT       1
#define MOEDA_25_CENT       2
#define MOEDA_10_CENT       3
#define MOEDA_10_CENT_GOLD  4
#define MOEDA_5_CENT        5
#define MOEDA_5_CENT_BRONZE 6

/* COIN PIXEL AMOUNTS */
#define MOEDA_1_REAL_PIXEL          117000
#define MOEDA_50_CENT_PIXEL         83000
#define MOEDA_25_CENT_PIXEL         100000
#define MOEDA_10_CENT_PIXEL         0
#define MOEDA_10_CENT_GOLD_PIXEL    63117
#define MOEDA_5_CENT_PIXEL          0
#define MOEDA_5_CENT_BRONZE_PIXEL   75000

/* ERROR MARGIN */
#define MARGEM_ERRO 0.075

/* ITK Definitions */
typedef itk::Image<unsigned char, 2>  ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>  BinaryThresholdImageFilterType;
typedef itk::BinaryCrossStructuringElement<ImageType::PixelType, ImageType::ImageDimension> StructuringElementType;
typedef itk::BinaryMorphologicalClosingImageFilter <ImageType, ImageType, StructuringElementType> BinaryMorphologicalClosingImageFilterType;
typedef itk::InvertIntensityImageFilter <ImageType> InvertIntensityImageFilterType;
typedef itk::BinaryImageToLabelMapFilter<ImageType> BinaryImageToLabelMapFilterType;

/* @FUNCTIONS    */

/* Create reader from file */
ReaderType::Pointer readFromFile(char* path) {
    printf("> Reading file: %s... ", path);
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(path);
    reader->Update();
    printf("[DONE]\n");
    
    return reader;
}

/* Binary threshold filter */
BinaryThresholdImageFilterType::Pointer applyThresholdFilter(ImageType::Pointer src, int lowerThreshold, int upperThreshold, int insideValue, int outsideValue) {
    printf("> Applying Threshold filter... ");
    BinaryThresholdImageFilterType::Pointer thresholdFilter  = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(src);
    thresholdFilter->SetLowerThreshold(lowerThreshold);
    thresholdFilter->SetUpperThreshold(upperThreshold);
    thresholdFilter->SetInsideValue(insideValue);
    thresholdFilter->SetOutsideValue(outsideValue);
    printf("[DONE]\n");
    
    return thresholdFilter;
}

/* Binary morphological filter */
BinaryMorphologicalClosingImageFilterType::Pointer applyMorphologicalClosingFilter(ImageType::Pointer src, int radius) {
    printf("> Applying MorphologicalClosing filter... ");
    // Create structure
    StructuringElementType structuringElement;
    structuringElement.SetRadius(radius);
    structuringElement.CreateStructuringElement();
    // Run filter    
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter  = BinaryMorphologicalClosingImageFilterType::New();
    closingFilter->SetInput(src);
    closingFilter->SetKernel(structuringElement);
    closingFilter->Update();
    printf("[DONE]\n");
    
    return closingFilter;
}

/* Invert image */
InvertIntensityImageFilterType::Pointer invertImage(ImageType::Pointer src, int maximum) {
    printf("> Applying Invert filter... ");
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(src);
    invertIntensityFilter->SetMaximum(maximum);
    printf("[DONE]\n");
    
    return invertIntensityFilter;
}

/* Create LabelMap from image */
BinaryImageToLabelMapFilterType::Pointer getLabelMap(ImageType::Pointer src) {
    printf("> Creating LabelMap... ");
    BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
    binaryImageToLabelMapFilter->SetInput(src);
    binaryImageToLabelMapFilter->Update();
    printf("[DONE]\n");
    
    return binaryImageToLabelMapFilter;
}

/* Comparison using an error margin */
bool compare(long int size, long int compareValue){ 
    if(size < (compareValue + compareValue*MARGEM_ERRO) && size > (compareValue - compareValue*MARGEM_ERRO)) {
        return true;
    } else {
        return false;
    }
}
  
/* Compare a given pixel amount with the defined pixel amounts for each coin */
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

/* @MAIN */
int main(int argc, char *argv[]){

    /* Check arguments */
    if(argc < 2) {
        printf("Usage: %s [FILE PATH]\n", argv[0]); 
        return 1;
    }
    
    /* Read input file */
    ReaderType::Pointer reader = readFromFile(argv[1]);
 
    /* Use a threshold filter to create a binary image */
    BinaryThresholdImageFilterType::Pointer thresholdFilter  = applyThresholdFilter(reader->GetOutput(), 10, 100, 255, 0);
  
    /* Apply a binary morphological closing filter to remove noise */
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter = applyMorphologicalClosingFilter(thresholdFilter->GetOutput(), 30);
    
    /* Invert image */
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = invertImage(closingFilter->GetOutput(), 255);

    /* Apply an imagetoLabelMap filter to separate objects */
    BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = getLabelMap(invertIntensityFilter->GetOutput());

    /* Loop over each region in the map */
    for(unsigned int i = 0; i < binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
        // Get the ith region
        BinaryImageToLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToLabelMapFilter->GetOutput()->GetNthLabelObject(i);
        labelObject->Optimize();
        
        // Check if the object size matches any coin
        printf("Object %10d - Size: %20ld - Type: %20s\n", i+1, labelObject->Size(), findCoinType(labelObject->Size()));
    }

    return EXIT_SUCCESS;
}   
  
  


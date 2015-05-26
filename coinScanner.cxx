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
#include "itkBinaryImageToShapeLabelMapFilter.h"
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

/* COIN LENGTH */
#define MOEDA_1_REAL_LENGTH         390
#define MOEDA_50_CENT_LENGTH        340
#define MOEDA_25_CENT_LENGTH        360
#define MOEDA_10_CENT_LENGTH        0
#define MOEDA_10_CENT_GOLD_LENGTH   286
#define MOEDA_5_CENT_LENGTH         0
#define MOEDA_5_CENT_BRONZE_LENGTH  315

/* ERROR MARGIN */
#define MARGEM_ERRO 0.075

/* OPTIONS */
#define USE_LABELMAP 1
#define USE_SHAPELABELMAP 1
#define SHOW_ALL_OUTPUT 0

/* ITK Definitions */
typedef itk::Image<unsigned char, 2>  ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>  BinaryThresholdImageFilterType;
typedef itk::BinaryCrossStructuringElement<ImageType::PixelType, ImageType::ImageDimension> StructuringElementType;
typedef itk::BinaryMorphologicalClosingImageFilter <ImageType, ImageType, StructuringElementType> BinaryMorphologicalClosingImageFilterType;
typedef itk::InvertIntensityImageFilter <ImageType> InvertIntensityImageFilterType;
typedef itk::BinaryImageToLabelMapFilter<ImageType> BinaryImageToLabelMapFilterType;
typedef itk::BinaryImageToShapeLabelMapFilter<ImageType> BinaryImageToShapeLabelMapFilterType;

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

BinaryImageToShapeLabelMapFilterType::Pointer getShapeLabelMap(ImageType::Pointer src) {
    printf("> Creating ShapeLabelMap... ");
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetInput(src);
    binaryImageToShapeLabelMapFilter->Update();
    printf("[DONE]\n");
    
    return binaryImageToShapeLabelMapFilter;
}

/* Comparison using an error margin */
long int compare(long int size, long int compareValue){ 
    if(size < (compareValue + compareValue*MARGEM_ERRO) && size > (compareValue - compareValue*MARGEM_ERRO)) {
        long int diff = size - compareValue;
        if(diff < 0) {
            diff *= -1;
        }
        if(diff == 0) {
            diff = 1;
        }
        return diff;
    } else {
        return 100000000;
    }
}
  
/* Compare a given pixel amount with the defined pixel amounts for each coin */
char* findCoinTypeSize(long int size) {
    long int closest = 100000000;
    char* closest_string = NULL;
    if(compare(size, MOEDA_1_REAL_PIXEL) < closest) {
        closest = compare(size, MOEDA_1_REAL_PIXEL);
        closest_string = (char *) "1 REAL";
    }
    if(compare(size, MOEDA_50_CENT_PIXEL) < closest) {
        closest = compare(size, MOEDA_50_CENT_PIXEL);
        closest_string = (char *) "50 CENTAVOS";
    }
    if(compare(size, MOEDA_25_CENT_PIXEL) < closest) {
        closest = compare(size, MOEDA_25_CENT_PIXEL);
        closest_string = (char *) "25 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_PIXEL) < closest) {
        closest = compare(size, MOEDA_10_CENT_PIXEL);
        closest_string = (char *) "10 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_GOLD_PIXEL) < closest) {
        closest = compare(size, MOEDA_10_CENT_GOLD_PIXEL);
        closest_string = (char *) "10 CENTAVOS GOLD";
    }
    if(compare(size, MOEDA_5_CENT_PIXEL) < closest) {
        closest = compare(size, MOEDA_5_CENT_PIXEL);
        closest_string = (char *) "5 CENTAVOS";
    }
    if(compare(size, MOEDA_5_CENT_BRONZE_PIXEL) < closest) {
        closest = compare(size, MOEDA_5_CENT_BRONZE_PIXEL);
        closest_string = (char *) "5 CENTAVOS BRONZE";
    }
    return closest_string;
}

/* Compare a given length with the defined lengths for each coin */
char* findCoinTypeLength(long int size) {
    long int closest = 100000000;
    char* closest_string = NULL;
    if(compare(size, MOEDA_1_REAL_LENGTH) < closest) {
        closest = compare(size, MOEDA_1_REAL_LENGTH);
        closest_string = (char *) "1 REAL";
    }
    if(compare(size, MOEDA_50_CENT_LENGTH) < closest) {
        closest = compare(size, MOEDA_50_CENT_LENGTH);
        closest_string = (char *) "50 CENTAVOS";
    }
    if(compare(size, MOEDA_25_CENT_LENGTH) < closest) {
        closest = compare(size, MOEDA_25_CENT_LENGTH);
        closest_string = (char *) "25 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_LENGTH) < closest) {
        closest = compare(size, MOEDA_10_CENT_LENGTH);
        closest_string = (char *) "10 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_GOLD_LENGTH) < closest) {
        closest = compare(size, MOEDA_10_CENT_GOLD_LENGTH);
        closest_string = (char *) "10 CENTAVOS GOLD";
    }
    if(compare(size, MOEDA_5_CENT_LENGTH) < closest) {
        closest = compare(size, MOEDA_5_CENT_LENGTH);
        closest_string = (char *) "5 CENTAVOS";
    }
    if(compare(size, MOEDA_5_CENT_BRONZE_LENGTH) < closest) {
        closest = compare(size, MOEDA_5_CENT_BRONZE_LENGTH);
        closest_string = (char *) "5 CENTAVOS BRONZE";
    }
    return closest_string;
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
    
    ImageType::Pointer image = reader->GetOutput();
 
    /* Use a threshold filter to create a binary image */
    BinaryThresholdImageFilterType::Pointer thresholdFilter  = applyThresholdFilter(reader->GetOutput(), 10, 100, 255, 0);
  
    /* Apply a binary morphological closing filter to remove noise */
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter = applyMorphologicalClosingFilter(thresholdFilter->GetOutput(), 30);
    
    /* Invert image */
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = invertImage(closingFilter->GetOutput(), 255);

    // Shape Label Map filter
    if(USE_LABELMAP) {
        /* Apply an imagetoLabelMap filter to separate objects */
        BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = getLabelMap(invertIntensityFilter->GetOutput());
        printf("> Results: \n");
        
        /* Loop over each region in the map */
        for(unsigned int i = 0; i < binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
            // Get the ith region
            BinaryImageToLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToLabelMapFilter->GetOutput()->GetNthLabelObject(i);
            labelObject->Optimize();
            
            // Check if the object size matches any coin
            char* objectType;
            objectType = findCoinTypeSize(labelObject->Size());
            if(SHOW_ALL_OUTPUT) {
                if(objectType == NULL) {
                    objectType = (char*) "INDEFINIDO";
                }
                printf("   Object %10d - Size: %20ld - Type: %20s\n", i+1, labelObject->Size(), objectType);
            } else {
                if(objectType != NULL) {
                    printf("   Object %10d - Size: %20ld - Type: %20s\n", i+1, labelObject->Size(), objectType);
                }
            }
        }
    }
    
    // Shape Label Map filter
    if(USE_SHAPELABELMAP) {
        /* Apply an imagetoShapeLabelMap filter to separate objects */
        BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = getShapeLabelMap(invertIntensityFilter->GetOutput());
        printf("> Results: \n");
        
        /* Loop over each region in the map */
        for(unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
            // Get the ith region
            BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
            
            // Check if the object size matches any coin
            char* objectType;
            objectType = findCoinTypeLength(labelObject->GetBoundingBox().GetSize()[0]);
            if(SHOW_ALL_OUTPUT) {
                if(objectType == NULL) {
                    objectType = (char*) "INDEFINIDO";
                }
                printf("   Object %10d - Length: %18ld - Type: %20s\n", i+1, labelObject->GetBoundingBox().GetSize()[0], objectType);
            } else {
                if(objectType != NULL) {
                    printf("   Object %10d - Length: %18ld - Type: %20s\n", i+1, labelObject->GetBoundingBox().GetSize()[0], objectType);
                }
            }
            
            /*for(unsigned int r = 0; r < labelObject->GetBoundingBox().GetSize()[0]; r++){
                for(unsigned int c = 0; c < labelObject->GetBoundingBox().GetSize()[1]; c++){
                  ImageType::IndexType pixelIndex;
                  pixelIndex[0] = labelObject->GetBoundingBox().GetIndex()[0] + r;
                  pixelIndex[1] = labelObject->GetBoundingBox().GetIndex()[1] + c;

                  image->SetPixel(pixelIndex, 0);
                }
            }*/
        }
    }
    
    //typedef  itk::ImageFileWriter< ImageType  > WriterType;
    //WriterType::Pointer writer = WriterType::New();
    //writer->SetFileName("output.png");
    //writer->SetInput(image);
    //writer->Update();

    return EXIT_SUCCESS;
}   
  
  


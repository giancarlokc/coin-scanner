#include "itkImage.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryImageToLabelMapFilter.h"
#include <stdio.h>

#define MOEDA_1_REAL        0
#define MOEDA_50_CENT       1
#define MOEDA_25_CENT       2
#define MOEDA_10_CENT       3
#define MOEDA_10_CENT_GOLD  4
#define MOEDA_5_CENT        5
#define MOEDA_5_CENT_BRONZE 6

#define MOEDA_1_REAL_PIXEL          1135013
#define MOEDA_50_CENT_PIXEL         0
#define MOEDA_25_CENT_PIXEL         0
#define MOEDA_10_CENT_PIXEL         63117
#define MOEDA_10_CENT_GOLD_PIXEL    0
#define MOEDA_5_CENT_PIXEL          0
#define MOEDA_5_CENT_BRONZE_PIXEL   0

#define MARGEM_ERRO 0.05

  typedef itk::Image<unsigned char, 2>  ImageType;
  typedef itk::Image<unsigned char, 2>  ImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>  BinaryThresholdImageFilterType;
  static void CreateImage(ImageType::Pointer image);
  
  bool compare(long int size, long int compareValue){ 
      if(size < (compareValue + compareValue*MARGEM_ERRO) && size > (compareValue - compareValue*MARGEM_ERRO)) {
           return true;
        } else {
          return false;
           }
      }
  

char* findCoinType(long int size) {
    if(compare(size, MOEDA_1_REAL_PIXEL)) {
        return "1 REAL";
    }
    if(compare(size, MOEDA_50_CENT_PIXEL)) {
        return "50 CENTAVOS";
    }
    if(compare(size, MOEDA_25_CENT_PIXEL)) {
        return "25 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_PIXEL)) {
        return "10 CENTAVOS";
    }
    if(compare(size, MOEDA_10_CENT_GOLD_PIXEL)) {
        return "10 CENTAVOS GOLD";
    }
    if(compare(size, MOEDA_5_CENT_PIXEL)) {
        return "5 CENTAVOS";
    }
    if(compare(size, MOEDA_5_CENT_BRONZE_PIXEL)) {
        return "5 CENTAVOS BRONZE";
    }
    return "INDEFINIDO";
}
  
  

int main(int argc, char *argv[]){

  ImageType::Pointer image;
  ImageType::Pointer image1;

  if(argc < 1)
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " InputImageFile " << std::endl;
    return EXIT_FAILURE;
    }
  
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(argv[1]);
  reader->Update();
 
  
  int lowerThreshold = 10;
  int upperThreshold = 100;
  unsigned int radius = 30;
 
  BinaryThresholdImageFilterType::Pointer thresholdFilter  = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(reader->GetOutput());
  thresholdFilter->SetLowerThreshold(lowerThreshold);
  thresholdFilter->SetUpperThreshold(upperThreshold);
  thresholdFilter->SetInsideValue(0);
  thresholdFilter->SetOutsideValue(255);
  
  image = thresholdFilter->GetOutput();
  

  std::cout << "Radius: " << radius << std::endl;
  typedef itk::BinaryBallStructuringElement<ImageType::PixelType, ImageType::ImageDimension> StructuringElementType;
  StructuringElementType structuringElement;
  structuringElement.SetRadius(radius);
  structuringElement.CreateStructuringElement();

  typedef itk::BinaryMorphologicalClosingImageFilter <ImageType, ImageType, StructuringElementType> BinaryMorphologicalClosingImageFilterType;
  BinaryMorphologicalClosingImageFilterType::Pointer closingFilter  = BinaryMorphologicalClosingImageFilterType::New();
  closingFilter->SetInput(image);
  closingFilter->SetKernel(structuringElement);
  closingFilter->Update();

  image1 = closingFilter->GetOutput();



    //Use the imagetoLabelMap filter
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
  
  


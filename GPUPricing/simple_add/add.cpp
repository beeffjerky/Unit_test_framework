#include <iostream>
#include <CL/cl.hpp>
 
int main(){
  //get all platforms (drivers)
  std::vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);
  if(all_platforms.size()==0){
    std::cout<<" No platforms found. Check OpenCL installation!\n";
    exit(1);
  }
  cl::Platform default_platform=all_platforms[0];
  std::cout << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";
 
  //get default device of the default platform
  std::vector<cl::Device> all_devices;
  default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
  if(all_devices.size()==0){
    std::cout<<" No devices found. Check OpenCL installation!\n";
    exit(1);
  }
  cl::Device default_device=all_devices[0];
  std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<"\n";
 
  VECTOR_CLASS<cl::Device> devices;
  devices.push_back(default_device);
  cl::Context context(devices);
 
  cl::Program::Sources sources;
 
  // kernel calculates for each element C=A+B
  std::string kernel_code=
    "  #pragma OPENCL EXTENSION cl_khr_fp64 : enable \n"
    "  typedef struct { \n"
    "    double ripBase; \n"
    "    double loss, reinstatementPrem, fullRip; \n"
    "    int riskGroup; \n"
    "    int eventId; \n"
    "    int sequenceId; \n"
    "    bool noncat; \n"
    "  } Event; \n"
    "  void kernel simple_add(global const Event* A, global const float4* B, global float* C){ \n"
    "    int id = get_global_id(0);  \n"
    "    //C[get_global_id(0)]=A[get_global_id(0)]+B[get_global_id(0)];  \n"
    "    C[id]=A[id].loss+B[id].x;   \n"
    "  }            ";
  sources.push_back({kernel_code.c_str(),kernel_code.length()});
 
  cl::Program program(context,sources);
  //if(program.build({default_device})!=CL_SUCCESS){
  if(program.build(devices)!=CL_SUCCESS){
    std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
    exit(1);
  }
 
  // create buffers on the device
  cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*10);
  cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*10);
  cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*10);
 
  int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
 
  //create queue to which we will push commands for the device.
  cl::CommandQueue queue(context,default_device);
 
  //write arrays A and B to the device
  queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
  queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);
 
  // run the kernel
  cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
  kernel_add.setArg(0,buffer_A);
  kernel_add.setArg(1,buffer_B);
  kernel_add.setArg(2,buffer_C);
  queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(10),cl::NullRange);
  queue.finish();

  //auto simple_add = cl::make_kernel<cl::Buffer&, cl::Buffer&, cl::Buffer&>(program, "simple_add");

 
  int C[10];
  //read result C from the device to array C
  queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*10,C);
 
  std::cout<<" result: \n";
  for(int i=0;i<10;i++)
    std::cout<<C[i]<<" ";
  std::cout << std::endl;
 
  return 0;
}

class Softmax 
    /*Expected Input Tensor Shape [N,1,1,W] in NCHW format in constructor
    N = BATCH_SIZE, W = flattened vector length
    Output is of same shape out = softmax(inp) 
    backprop expects dL/dout (grad_in) and returns dL/dinp
    L = any loss computed from output of softmax eg cross entropy 
    Note that we need to have another layer to compute loss if we like to calculate*/
{
    public:
        // Declare public variables
        const float alpha = 1.0f;
        const float beta = 0.0f;
        
        cudnnTensorDescriptor_t input_descriptor;
        cudnnTensorDescriptor_t output_descriptor;
        
        cudnnHandle_t cudnn;
        cublasHandle_t cublas;
 
        // These variables will be on GPU as cache for backward pass 
        float *dot;  //Output of softmax i.e., dot = softmax(d_input) in forward, necessary to cache for backward
 
        // These variables will be on CPU
        int input_size, output_size;
        int out_height, out_width;
        int in_channels, out_channels;
        int gpu_id;
        float *dot_cpu; //Cache for backprop
 
        Softmax(int _in_channels, int _out_channels, cudnnHandle_t _cudnn, cublasHandle_t _cublas,
             int batch_size, int height, int width, int _gpu_id)
        {
            cudnn = _cudnn;
            cublas = _cublas;
            gpu_id = _gpu_id;

            checkCudaErrors(cudaSetDevice(gpu_id));
         
            in_channels = _in_channels;
            out_channels = _out_channels;
            out_width = width;
            out_height = height;
         
            checkCUDNN(cudnnCreateTensorDescriptor(&input_descriptor));
            checkCUDNN(cudnnSetTensor4dDescriptor(input_descriptor, 
                                                      CUDNN_TENSOR_NCHW,
                                                      CUDNN_DATA_FLOAT,
                                                      batch_size,
                                                      in_channels,
                                                      height,
                                                      width));
                
            checkCUDNN(cudnnCreateTensorDescriptor(&output_descriptor));
            checkCUDNN(cudnnSetTensor4dDescriptor(output_descriptor,
                                                      CUDNN_TENSOR_NCHW,
                                                      CUDNN_DATA_FLOAT,
                                                      batch_size,
                                                      out_channels,
                                                      out_height,
                                                      out_width));
            
            // Allocate memory to GPU placeholders
            input_size = batch_size * in_channels * height * width;
            output_size = input_size; //output_size means output of softmax, not the scalar loss
         
            checkCudaErrors(cudaMalloc(&dot, sizeof(float) * output_size));
            dot_cpu = (float *)malloc(sizeof(float) * output_size);
        }
 
        void forward(float *d_input, float *d_output)
        {
            // Performs forward pass for softmax layer
            checkCUDNN(cudnnSoftmaxForward(
                cudnn,
                CUDNN_SOFTMAX_ACCURATE,
                CUDNN_SOFTMAX_MODE_INSTANCE,
                &alpha,
                input_descriptor,
                d_input,
                &beta,
                output_descriptor,
                d_output    
            ));
         
            //Store the output of softmax for backprop
            checkCudaErrors(cudaMemcpy(dot_cpu, d_output, sizeof(float) * output_size, cudaMemcpyDeviceToHost));
            checkCudaErrors(cudaMemcpy(dot, dot_cpu, sizeof(float) * output_size,  cudaMemcpyHostToDevice));
        }
 
        void backward(float *grad_above, float *grad_out)
        {
            // Performs backward pass for softmax activation
            checkCUDNN(cudnnSoftmaxBackward(
                cudnn,
                CUDNN_SOFTMAX_ACCURATE,
                CUDNN_SOFTMAX_MODE_INSTANCE,
                &alpha,
                output_descriptor,
                dot,
                output_descriptor,
                grad_above,
                &beta,
                input_descriptor,
                grad_out
            ));
        }
};

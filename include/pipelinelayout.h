#include "core.h"

class PipelineLayout {
private:
    const LogicalDevice* mDevice = nullptr;
    VkPipelineLayout mLayout = VK_NULL_HANDLE;

    PipelineLayout(const LogicalDevice* device, VkPipelineLayout layout);
    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout& operator=(const PipelineLayout&) = delete;
    PipelineLayout(PipelineLayout&&) = delete;
    PipelineLayout& operator=(PipelineLayout&&) = delete;
public:
    inline operator const VkPipelineLayout& () const { return mLayout; }

    static PPipelineLayout CreatePipelineLayout(const LogicalDevice* device);

    ~PipelineLayout();
};
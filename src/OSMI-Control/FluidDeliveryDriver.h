/**
 * @brief Abstract Driver for Fluid Delivery Driver
 * 
 */
class FluidDeliveryDriver {
    virtual FluidDeliveryError setFlowRate (int freq) = 0;

    virtual void disable() = 0;
    virtual void enable() = 0;

    virtual int getDistanceFB() = 0;
};

class FluidDeliveryError {
    virtual int getID() = 0;
};


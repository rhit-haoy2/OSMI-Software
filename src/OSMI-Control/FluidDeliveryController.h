class FluidDeliveryController {
    public:
        virtual void startFlow() = 0;
        virtual void stopFlow() = 0;

        /// @brief get the total volume delivered from the controller.
        /// @return the volume delivered in mL.
        virtual float getVolumeDelivered() = 0;

        FluidDeliveryController();
        virtual ~FluidDeliveryController() {}; 
};
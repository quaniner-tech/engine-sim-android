#include <gtest/gtest.h>
#include <engine_sim/engine.h>
#include <engine_sim/piston_engine_simulator.h>
#include <engine_sim/vehicle.h>
#include <engine_sim/transmission.h>

using namespace engine_sim;

// Helper: create an inline-4 engine with default parameters
static Engine createInline4Engine() {
    Engine engine;
    Engine::Parameters params;
    params.cylinderBanks = 1;
    params.cylinderCount = 4;
    params.crankshaftCount = 1;
    params.exhaustSystemCount = 1;
    params.intakeCount = 1;
    params.name = "Inline-4 Test Engine";
    params.starterTorque = 90.0 * 1.3558179483314; // N*m
    params.starterSpeed = 200.0 * 2.0 * 3.14159265359 / 60.0; // rad/s
    params.redline = 7000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoMinSpeed = 1000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoMaxSpeed = 7000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
    params.initialSimulationFrequency = 10000.0;
    params.initialHighFrequencyGain = 0.3;
    params.initialNoise = 0.05;
    params.initialJitter = 0.001;
    engine.initialize(params);
    return engine;
}

TEST(EngineTest, Construction) {
    Engine engine;
    EXPECT_TRUE(engine.getName().empty());
}

TEST(EngineTest, InitializeInline4) {
    Engine engine = createInline4Engine();
    EXPECT_EQ(engine.getName(), "Inline-4 Test Engine");
    EXPECT_EQ(engine.getCylinderCount(), 4);
    EXPECT_EQ(engine.getCylinderBankCount(), 1);
    EXPECT_EQ(engine.getCrankshaftCount(), 1);
}

TEST(EngineTest, InitializeV8) {
    Engine engine;
    Engine::Parameters params;
    params.cylinderBanks = 2;
    params.cylinderCount = 8;
    params.crankshaftCount = 1;
    params.exhaustSystemCount = 2;
    params.intakeCount = 2;
    params.name = "V8 Engine";
    params.starterTorque = 150.0 * 1.3558179483314;
    params.starterSpeed = 150.0 * 2.0 * 3.14159265359 / 60.0;
    params.redline = 7000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoMinSpeed = 700.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoMaxSpeed = 7000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
    params.initialSimulationFrequency = 10000.0;
    params.initialHighFrequencyGain = 0.4;
    params.initialNoise = 0.04;
    params.initialJitter = 0.001;
    engine.initialize(params);
    EXPECT_EQ(engine.getCylinderCount(), 8);
    EXPECT_EQ(engine.getCylinderBankCount(), 2);
}

TEST(EngineTest, ThrottleControl) {
    Engine engine = createInline4Engine();

    // Initially no throttle
    EXPECT_DOUBLE_EQ(engine.getThrottle(), 0.0);

    // Set throttle to 50%
    engine.setThrottle(0.5);
    EXPECT_DOUBLE_EQ(engine.getThrottle(), 0.5);

    // Set throttle to 100%
    engine.setThrottle(1.0);
    EXPECT_DOUBLE_EQ(engine.getThrottle(), 1.0);

    // Throttle plate angle should increase with throttle
    EXPECT_GE(engine.getThrottlePlateAngle(), 0.0);
}

TEST(EngineTest, SpeedControl) {
    Engine engine = createInline4Engine();

    // Set speed control
    engine.setSpeedControl(500.0); // rad/s
    EXPECT_DOUBLE_EQ(engine.getSpeedControl(), 500.0);

    // Changing speed control
    engine.setSpeedControl(300.0);
    EXPECT_DOUBLE_EQ(engine.getSpeedControl(), 300.0);
}

TEST(EngineTest, RedlineAndDynoRanges) {
    Engine engine = createInline4Engine();

    EXPECT_GE(engine.getRedline(), 0.0);
    EXPECT_GE(engine.getDynoMaxSpeed(), engine.getDynoMinSpeed());
    EXPECT_GE(engine.getDynoHoldStep(), 0.0);
    EXPECT_GE(engine.getStarterTorque(), 0.0);
    EXPECT_GE(engine.getStarterSpeed(), 0.0);
}

TEST(EngineTest, EngineGetters) {
    Engine engine = createInline4Engine();

    // Crankshaft access
    ASSERT_NE(engine.getOutputCrankshaft(), nullptr);

    // Cylinder bank access
    ASSERT_NE(engine.getCylinderBank(0), nullptr);

    // Fuel system access
    ASSERT_NE(engine.getFuel(), nullptr);

    // Ignition module access
    ASSERT_NE(engine.getIgnitionModule(), nullptr);
}

TEST(EngineTest, PistonAccess) {
    Engine engine = createInline4Engine();

    EXPECT_EQ(engine.getPiston(0), nullptr); // Pistons not yet connected
    EXPECT_GE(engine.getMaxDepth(), 0);
}

TEST(EngineTest, SimulationParameters) {
    Engine engine = createInline4Engine();

    EXPECT_GE(engine.getSimulationFrequency(), 0.0);
    EXPECT_GE(engine.getInitialHighFrequencyGain(), 0.0);
    EXPECT_GE(engine.getInitialNoise(), 0.0);
    EXPECT_GE(engine.getInitialJitter(), 0.0);
}

TEST(EngineTest, TorqueAndPowerGetters) {
    Engine engine = createInline4Engine();

    // Engine should be able to report power/torque without crashing
    double rpm = engine.getRpm();
    EXPECT_GE(rpm, 0.0);

    EXPECT_GE(engine.getManifoldPressure(), 0.0);
    EXPECT_GE(engine.getIntakeAfr(), 0.0);
    EXPECT_GE(engine.getExhaustO2(), 0.0);
    EXPECT_GE(engine.getSpeed(), 0.0);
}

TEST(EngineTest, SpinningState) {
    Engine engine = createInline4Engine();

    // Before simulation, might not be spinning
    bool spinning = engine.isSpinningCw();
    // No assertion on initial state - depends on initialization
    (void)spinning;
}

TEST(EngineTest, FuelConsumption) {
    Engine engine = createInline4Engine();

    engine.resetFuelConsumption();
    EXPECT_DOUBLE_EQ(engine.getTotalFuelMassConsumed(), 0.0);
    EXPECT_GE(engine.getTotalVolumeFuelConsumed(), 0.0);
}

TEST(EngineTest, EngineDisplacement) {
    Engine engine = createInline4Engine();

    double displacement = engine.getDisplacement();
    EXPECT_GE(displacement, 0.0);
}

TEST(PistonEngineSimulatorTest, CreateAndDestroy) {
    PistonEngineSimulator simulator;
    Engine* engine = new Engine();
    Vehicle* vehicle = new Vehicle();
    Transmission* transmission = new Transmission();

    Engine::Parameters params;
    params.cylinderBanks = 1;
    params.cylinderCount = 4;
    params.crankshaftCount = 1;
    params.exhaustSystemCount = 1;
    params.intakeCount = 1;
    params.name = "Test Engine";
    params.starterTorque = 90.0 * 1.3558179483314;
    params.starterSpeed = 200.0 * 2.0 * 3.14159265359 / 60.0;
    params.redline = 7000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoMinSpeed = 1000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoMaxSpeed = 7000.0 * 2.0 * 3.14159265359 / 60.0;
    params.dynoHoldStep = 100.0 * 2.0 * 3.14159265359 / 60.0;
    params.initialSimulationFrequency = 10000.0;
    params.initialHighFrequencyGain = 0.3;
    params.initialNoise = 0.05;
    params.initialJitter = 0.001;
    engine->initialize(params);

    simulator.loadSimulation(engine, vehicle, transmission);
    simulator.setSimulationFrequency(10000);

    // Should not crash
    EXPECT_NE(simulator.getEngine(), nullptr);
    EXPECT_NE(simulator.getVehicle(), nullptr);
    EXPECT_NE(simulator.getTransmission(), nullptr);

    simulator.destroy();
    delete engine;
    delete vehicle;
    delete transmission;
}

TEST(PistonEngineSimulatorTest, SimulationFrequency) {
    PistonEngineSimulator simulator;
    simulator.setSimulationFrequency(10000);
    EXPECT_EQ(simulator.getSimulationFrequency(), 10000);

    simulator.setSimulationFrequency(5000);
    EXPECT_EQ(simulator.getSimulationFrequency(), 5000);
}

TEST(PistonEngineSimulatorTest, Timestep) {
    PistonEngineSimulator simulator;
    simulator.setSimulationFrequency(10000);
    EXPECT_DOUBLE_EQ(simulator.getTimestep(), 0.0001);

    simulator.setSimulationFrequency(5000);
    EXPECT_DOUBLE_EQ(simulator.getTimestep(), 0.0002);
}
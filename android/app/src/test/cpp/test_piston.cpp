#include <gtest/gtest.h>
#include <engine_sim/piston.h>
#include <engine_sim/cylinder_bank.h>
#include <engine_sim/connecting_rod.h>

using namespace engine_sim;

// Minimal mock to allow Piston construction
class MockCylinderBank : public CylinderBank {
public:
    MockCylinderBank() {
        // Minimal setup for testing
    }
};

class MockConnectingRod : public ConnectingRod {
public:
    MockConnectingRod() {
        // Minimal setup for testing
    }
};

TEST(PistonTest, Construction) {
    Piston piston;
    // No crash on construction
}

TEST(PistonTest, PistonGetters) {
    Piston piston;

    // Before initialization, getters return default values
    EXPECT_DOUBLE_EQ(piston.getCompressionHeight(), 0.0);
    EXPECT_DOUBLE_EQ(piston.getDisplacement(), 0.0);
    EXPECT_EQ(piston.getCylinderIndex(), 0);
    EXPECT_EQ(piston.getRod(), nullptr);
    EXPECT_EQ(piston.getCylinderBank(), nullptr);
}

TEST(PistonTest, PistonMassAndBlowby) {
    Piston piston;

    // Mass and blowby coefficient
    EXPECT_GE(piston.getMass(), 0.0);
    EXPECT_GE(piston.getBlowbyK(), 0.0);
}

TEST(PistonTest, RelativePosition) {
    Piston piston;

    // Relative position should be defined (may be 0 before initialization)
    double relX = piston.relativeX();
    double relY = piston.relativeY();
    // These can be any value - just verify no crash
    (void)relX;
    (void)relY;
}

TEST(PistonTest, InitializeWithParams) {
    Piston piston;
    Piston::Parameters params;
    params.CylinderIndex = 2;
    params.CompressionHeight = 0.05; // 50mm
    params.WristPinPosition = 0.025;
    params.Displacement = 0.0005; // 500cc
    params.mass = 0.5; // 500g
    params.BlowbyFlowCoefficient = 0.001;
    // Note: Rod and Bank set to nullptr for unit test isolation

    piston.initialize(params);

    EXPECT_EQ(piston.getCylinderIndex(), 2);
    EXPECT_DOUBLE_EQ(piston.getCompressionHeight(), 0.05);
    EXPECT_DOUBLE_EQ(piston.getDisplacement(), 0.0005);
    EXPECT_GE(piston.getMass(), 0.0);
}

TEST(PistonTest, CalculateCylinderWallForce) {
    Piston piston;
    Piston::Parameters params;
    params.CylinderIndex = 0;
    params.CompressionHeight = 0.05;
    params.WristPinPosition = 0.025;
    params.Displacement = 0.0005;
    params.mass = 0.5;
    params.BlowbyFlowCoefficient = 0.001;
    piston.initialize(params);

    // Force calculation should not crash
    double force = piston.calculateCylinderWallForce();
    // Force should be >= 0 (physical constraint force)
    EXPECT_GE(force, 0.0);
}

TEST(PistonTest, WristPinLocation) {
    Piston piston;
    Piston::Parameters params;
    params.CylinderIndex = 1;
    params.CompressionHeight = 0.06; // 60mm
    params.WristPinPosition = 0.03; // centered
    params.Displacement = 0.0006;
    params.mass = 0.6;
    params.BlowbyFlowCoefficient = 0.001;
    piston.initialize(params);

    double wristPin = piston.getWristPinLocation();
    EXPECT_DOUBLE_EQ(wristPin, 0.03);
}

TEST(PistonTest, MultiplePistons) {
    // Create multiple pistons with different indices
    Piston pistons[4];
    for (int i = 0; i < 4; ++i) {
        Piston::Parameters params;
        params.CylinderIndex = i;
        params.CompressionHeight = 0.05 + i * 0.01;
        params.WristPinPosition = 0.025 + i * 0.005;
        params.Displacement = 0.0005 + i * 0.0001;
        params.mass = 0.5 + i * 0.1;
        params.BlowbyFlowCoefficient = 0.001;
        pistons[i].initialize(params);

        EXPECT_EQ(pistons[i].getCylinderIndex(), i);
        EXPECT_DOUBLE_EQ(pistons[i].getCompressionHeight(), 0.05 + i * 0.01);
    }
}

TEST(PistonTest, Destroy) {
    Piston piston;
    Piston::Parameters params;
    params.CylinderIndex = 0;
    params.CompressionHeight = 0.05;
    params.WristPinPosition = 0.025;
    params.Displacement = 0.0005;
    params.mass = 0.5;
    params.BlowbyFlowCoefficient = 0.001;
    piston.initialize(params);

    // Should not crash
    piston.destroy();
}
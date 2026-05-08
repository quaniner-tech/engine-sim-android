package com.enginesim

import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.mockito.Mock
import org.mockito.MockitoAnnotations

/**
 * Unit tests for EngineSimLibrary Kotlin wrapper.
 * Uses MockK for mocking native calls if needed.
 */
class EngineSimLibraryTest {

    private lateinit var library: EngineSimLibrary

    @Before
    fun setup() {
        library = EngineSimLibrary()
    }

    @Test
    fun `initial state - not created`() {
        assertFalse("Library should not be created initially", library.isCreated)
    }

    @Test
    fun `create - should return true and mark as created`() {
        val result = library.create()
        assertTrue("create() should succeed", result)
        assertTrue("isCreated should be true after successful create", library.isCreated)

        // Clean up
        library.destroy()
    }

    @Test
    fun `create twice - should destroy first and create new`() {
        val first = library.create()
        assertTrue(first)
        val handle1 = library.isCreated

        // Create again - should replace
        val second = library.create()
        assertTrue("second create() should succeed", second)
        assertTrue("should still be created", library.isCreated)
    }

    @Test
    fun `destroy - should mark as not created`() {
        library.create()
        assertTrue(library.isCreated)

        library.destroy()
        assertFalse("should not be created after destroy", library.isCreated)
    }

    @Test
    fun `getRpm - returns 0 when not created`() {
        val rpm = library.getRpm()
        assertEquals("RPM should be 0 when not created", 0f, rpm, 0.001f)
    }

    @Test
    fun `getRpm - returns value when created`() {
        library.create()
        val rpm = library.getRpm()
        // RPM should be non-negative after creation
        assertTrue("RPM should be non-negative", rpm >= 0f)
        library.destroy()
    }

    @Test
    fun `getTorque - returns 0 when not created`() {
        val torque = library.getTorque()
        assertEquals("Torque should be 0 when not created", 0f, torque, 0.001f)
    }

    @Test
    fun `getPower - returns 0 when not created`() {
        val power = library.getPower()
        assertEquals("Power should be 0 when not created", 0f, power, 0.001f)
    }

    @Test
    fun `getSpeed - returns 0 when not created`() {
        val speed = library.getSpeed()
        assertEquals("Speed should be 0 when not created", 0f, speed, 0.001f)
    }

    @Test
    fun `isSpinning - returns false when not created`() {
        assertFalse("Should not be spinning when not created", library.isSpinning())
    }

    @Test
    fun `setThrottle - does not crash when not created`() {
        // Should be safe to call even when not created (guards against null handle)
        library.setThrottle(0.5f)
        // No exception thrown
    }

    @Test
    fun `setThrottle - does not crash when created`() {
        library.create()
        library.setThrottle(0.5f)
        library.setThrottle(1.0f)
        library.setThrottle(0.0f)
        library.destroy()
    }

    @Test
    fun `setSpeedControl - does not crash when not created`() {
        library.setSpeedControl(500f)
    }

    @Test
    fun `start and stop - do not crash`() {
        library.create()
        library.start()
        library.stop()
        library.destroy()
    }

    @Test
    fun `readAudio - returns 0 when not created`() {
        val buffer = ShortArray(256)
        val read = library.readAudio(buffer, 256)
        assertEquals("Should read 0 samples when not created", 0, read)
    }

    @Test
    fun `readAudio - returns samples when created`() {
        library.create()
        val buffer = ShortArray(1024)
        val read = library.readAudio(buffer, 1024)
        // read may be 0 or positive (depends on audio thread state)
        assertTrue("read should be >= 0", read >= 0)
        assertTrue("read should be <= requested", read <= 1024)
        library.destroy()
    }

    @Test
    fun `setVolume - does not crash`() {
        library.create()
        library.setVolume(0.5f)
        library.setVolume(1.0f)
        library.setVolume(0.0f)
        library.setVolume(1.5f) // Can exceed 1.0
        library.destroy()
    }

    @Test
    fun `getSampleRate - returns default when not created`() {
        val sr = library.getSampleRate()
        assertEquals("Default sample rate should be 44100", 44100, sr)
    }

    @Test
    fun `getSampleRate - returns value when created`() {
        library.create()
        val sr = library.getSampleRate()
        assertTrue("Sample rate should be positive", sr > 0)
        library.destroy()
    }

    @Test
    fun `simulateStep - returns false when not created`() {
        val result = library.simulateStep()
        assertFalse("simulateStep should return false when not created", result)
    }

    @Test
    fun `simulateStep - returns value when created`() {
        library.create()
        val result = library.simulateStep()
        // Result depends on engine state
        assertNotNull("result should not be null", result)
        library.destroy()
    }

    @Test
    fun `getExhaustFlow - returns 0 when not created`() {
        val flow = library.getExhaustFlow()
        assertEquals("Exhaust flow should be 0 when not created", 0.0, flow, 0.001)
    }

    @Test
    fun `loadEnginePreset Inline4 - returns true when created`() {
        library.create()
        val result = library.loadEnginePreset(EngineSimLibrary.EnginePreset.Inline4)
        assertTrue("loadEnginePreset should return true", result)
        library.destroy()
    }

    @Test
    fun `loadEnginePreset V6 - returns true when created`() {
        library.create()
        val result = library.loadEnginePreset(EngineSimLibrary.EnginePreset.V6)
        assertTrue("loadEnginePreset should return true", result)
        library.destroy()
    }

    @Test
    fun `loadEnginePreset V8 - returns true when created`() {
        library.create()
        val result = library.loadEnginePreset(EngineSimLibrary.EnginePreset.V8)
        assertTrue("loadEnginePreset should return true", result)
        library.destroy()
    }

    @Test
    fun `loadEnginePreset V12 - returns true when created`() {
        library.create()
        val result = library.loadEnginePreset(EngineSimLibrary.EnginePreset.V12)
        assertTrue("loadEnginePreset should return true", result)
        library.destroy()
    }

    @Test
    fun `loadEngineConfig - returns false with empty config`() {
        library.create()
        val result = library.loadEngineConfig("")
        // Empty config may fail or return false
        assertFalse("empty config should return false", result)
        library.destroy()
    }

    @Test
    fun `loadEngineConfig - returns false when not created`() {
        val result = library.loadEngineConfig("{}")
        assertFalse("loadEngineConfig should return false when not created", result)
    }

    @Test
    fun `loadEnginePreset - returns false when not created`() {
        val result = library.loadEnginePreset(EngineSimLibrary.EnginePreset.Inline4)
        assertFalse("loadEnginePreset should return false when not created", result)
    }

    @Test
    fun `full lifecycle - create, configure, simulate, destroy`() {
        // Create
        assertTrue(library.create())

        // Configure
        library.loadEnginePreset(EngineSimLibrary.EnginePreset.Inline4)
        library.setThrottle(0.5f)
        library.setVolume(0.8f)

        // Simulate
        for (i in 0..9) {
            library.simulateStep()
            val rpm = library.getRpm()
            assertTrue("RPM should be non-negative", rpm >= 0f)
        }

        // Read audio
        val buffer = ShortArray(512)
        library.readAudio(buffer, 512)

        // State checks
        assertTrue(library.isCreated)
        assertTrue(library.getRpm() >= 0f)

        // Destroy
        library.destroy()
        assertFalse(library.isCreated)
    }
}
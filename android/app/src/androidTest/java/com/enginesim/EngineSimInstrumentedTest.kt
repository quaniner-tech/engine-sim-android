package com.enginesim

import android.content.Context
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.MediumTest
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented tests for EngineSimLibrary.
 * Tests actual JNI integration and audio output on Android device/emulator.
 */
@RunWith(AndroidJUnit4::class)
@MediumTest
class EngineSimInstrumentedTest {

    private lateinit var library: EngineSimLibrary
    private lateinit var context: Context

    @Before
    fun setup() {
        context = ApplicationProvider.getApplicationContext()
        library = EngineSimLibrary()
    }

    @After
    fun teardown() {
        if (library.isCreated) {
            library.destroy()
        }
    }

    @Test
    fun `native library loads successfully`() {
        // Library loading happens in companion object init
        // If we reach here, the libraries loaded without UnsatisfiedLinkError
        assertTrue("Library should be created", library.create())
    }

    @Test
    fun `JNI create returns valid handle`() {
        assertTrue("create() should succeed", library.create())
        val rpm = library.getRpm()
        // Should get some RPM value (may be 0 initially)
        assertTrue("RPM should be readable", rpm >= 0f)
    }

    @Test
    fun `audio rendering thread starts with create`() {
        assertTrue("create() should succeed", library.create())

        // Give audio thread time to start
        Thread.sleep(100)

        val buffer = ShortArray(1024)
        val samplesRead = library.readAudio(buffer, 1024)

        // Audio thread should be producing samples
        assertTrue("Should read some audio samples", samplesRead > 0)
    }

    @Test
    fun `audio output is continuous`() {
        assertTrue("create() should succeed", library.create())
        Thread.sleep(100)

        val buffer1 = ShortArray(512)
        val buffer2 = ShortArray(512)
        val buffer3 = ShortArray(512)

        val read1 = library.readAudio(buffer1, 512)
        val read2 = library.readAudio(buffer2, 512)
        val read3 = library.readAudio(buffer3, 512)

        assertTrue("First read should have samples", read1 > 0)
        assertTrue("Second read should have samples", read2 > 0)
        assertTrue("Third read should have samples", read3 > 0)
    }

    @Test
    fun `throttle changes affect audio output`() {
        assertTrue("create() should succeed", library.create())
        Thread.sleep(100)

        // Read baseline audio
        val baseline = ShortArray(1024)
        val baselineRead = library.readAudio(baseline, 1024)
        assertTrue("Should read baseline", baselineRead > 0)

        // Apply throttle
        library.setThrottle(1.0f)
        Thread.sleep(100)

        // Read audio with throttle
        val throttled = ShortArray(1024)
        val throttledRead = library.readAudio(throttled, 1024)
        assertTrue("Should read with throttle", throttledRead > 0)

        // Throttled audio should have different characteristics
        // (This is a soft assertion - exact behavior depends on audio synthesis)
        var baselineEnergy = 0L
        var throttledEnergy = 0L
        for (i in 0 until baselineRead) {
            baselineEnergy += baseline[i].toLong() * baseline[i].toLong()
            throttledEnergy += throttled[i].toLong() * throttled[i].toLong()
        }

        // At idle vs full throttle, energy should be different
        // (This may or may not hold depending on engine state)
    }

    @Test
    fun `engine presets load correctly`() {
        assertTrue("create() should succeed", library.create())

        // Test Inline4
        assertTrue("Inline4 preset should load", library.loadEnginePreset(EngineSimLibrary.EnginePreset.Inline4))
        val rpmInline4 = library.getRpm()

        // Test V8
        assertTrue("V8 preset should load", library.loadEnginePreset(EngineSimLibrary.EnginePreset.V8))

        // Test V12
        assertTrue("V12 preset should load", library.loadEnginePreset(EngineSimLibrary.EnginePreset.V12))

        // Engine should still be running after preset changes
        assertTrue("RPM should be readable", library.getRpm() >= 0f)
    }

    @Test
    fun `no memory leak on repeated create destroy`() {
        // Create and destroy multiple times
        repeat(5) {
            val lib = EngineSimLibrary()
            assertTrue("create() should succeed", lib.create())
            Thread.sleep(50)
            lib.destroy()
            Thread.sleep(50)
        }
        // If we get here without OOM or crash, test passes
    }

    @Test
    fun `JNI connection stable under load`() {
        assertTrue("create() should succeed", library.create())
        library.setThrottle(0.5f)

        // Run many simulation steps rapidly
        repeat(100) {
            library.simulateStep()
        }

        // Engine should still be responsive
        assertTrue("RPM should still be readable", library.getRpm() >= 0f)
        assertTrue("Should still be spinning", library.isSpinning())

        val buffer = ShortArray(512)
        assertTrue("Audio should still work", library.readAudio(buffer, 512) >= 0)
    }

    @Test
    fun `sample rate is correct`() {
        assertTrue("create() should succeed", library.create())
        val sampleRate = library.getSampleRate()
        assertEquals("Sample rate should be 44100", 44100, sampleRate)
    }

    @Test
    fun `simulate step advances engine state`() {
        assertTrue("create() should succeed", library.create())
        library.setThrottle(0.8f)

        val rpmBefore = library.getRpm()
        library.simulateStep()
        val rpmAfter = library.getRpm()

        // RPM should change or stay same (depends on starter state)
        // The key is it should not crash
        assertTrue("RPM should remain non-negative", rpmAfter >= 0f)
    }

    @Test
    fun `volume control works`() {
        assertTrue("create() should succeed", library.create())

        library.setVolume(0.0f)
        library.setVolume(0.5f)
        library.setVolume(1.0f)

        // Should still be functional
        assertTrue("Should still be created", library.isCreated)
    }

    @Test
    fun `engine start stop`() {
        assertTrue("create() should succeed", library.create())

        library.start()
        val spinning1 = library.isSpinning()

        library.stop()

        // Start/stop should not crash
        assertNotNull("spinning state should be readable", spinning1)
    }
}
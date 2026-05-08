package com.enginesim.app

import android.content.Context
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlin.math.cos
import kotlin.math.sin

class MainActivity : ComponentActivity() {
    private var rpm by mutableStateOf(800f)
    private var throttle by mutableStateOf(0f)
    private var volume by mutableStateOf(0.7f)
    private var torque by mutableStateOf(0f)
    private var power by mutableStateOf(0f)
    private var isRunning by mutableStateOf(false)
    private var selectedPreset by mutableStateOf("inline4")

    private lateinit var engineSimLibrary: EngineSimLibrary
    private var audioInitialized = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        engineSimLibrary = EngineSimLibrary()
        engineSimLibrary.create()

        setContent {
            MaterialTheme(colorScheme = darkColorScheme()) {
                Surface(color = Color(0xFF1A1A1A)) {
                    EngineDashboard(
                        rpm = rpm,
                        torque = torque,
                        power = power,
                        throttle = throttle,
                        volume = volume,
                        isRunning = isRunning,
                        selectedPreset = selectedPreset,
                        onThrottleChange = {
                            throttle = it
                            engineSimLibrary.setThrottle(it)
                        },
                        onVolumeChange = {
                            volume = it
                            engineSimLibrary.setVolume(it)
                        },
                        onPresetChange = { preset ->
                            selectedPreset = preset
                            engineSimLibrary.loadEnginePreset(preset, applicationContext)
                        },
                        onToggleEngine = {
                            if (isRunning) {
                                engineSimLibrary.stop()
                                isRunning = false
                            } else {
                                if (!audioInitialized) {
                                    audioInitialized = engineSimLibrary.initializeAudio(assetManager = assets)
                                }
                                engineSimLibrary.start()
                                isRunning = true
                                startSimulationLoop()
                            }
                        }
                    )
                }
            }
        }
    }

    private fun startSimulationLoop() {
        Thread {
            while (isRunning) {
                val running = engineSimLibrary.simulateStep()
                if (!running) {
                    isRunning = false
                    break
                }
                rpm = engineSimLibrary.getRpm()
                torque = engineSimLibrary.getTorque()
                power = engineSimLibrary.getPower()
                Thread.sleep(16)
            }
        }.start()
    }

    override fun onDestroy() {
        super.onDestroy()
        isRunning = false
        engineSimLibrary.destroy()
    }
}

@Composable
fun EngineDashboard(
    rpm: Float,
    torque: Float,
    power: Float,
    throttle: Float,
    volume: Float,
    isRunning: Boolean,
    selectedPreset: String,
    onThrottleChange: (Float) -> Unit,
    onVolumeChange: (Float) -> Unit,
    onPresetChange: (String) -> Unit,
    onToggleEngine: () -> Unit
) {
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(scrollState)
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // Title
        Text(
            text = "ENGINE SIMULATOR",
            fontSize = 28.sp,
            fontWeight = FontWeight.Bold,
            color = Color.White,
            modifier = Modifier.padding(bottom = 24.dp)
        )

        // RPM Gauge
        RpmGauge(rpm = rpm, modifier = Modifier.fillMaxWidth().height(200.dp))

        Spacer(modifier = Modifier.height(16.dp))

        // Torque / Power cards
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            StatCard(
                value = String.format("%.0f", torque),
                unit = "Nm",
                label = "TORQUE",
                modifier = Modifier.weight(1f)
            )
            StatCard(
                value = String.format("%.0f", power),
                unit = "HP",
                label = "POWER",
                modifier = Modifier.weight(1f)
            )
        }

        Spacer(modifier = Modifier.height(20.dp))

        // Throttle slider
        LabeledSlider(
            label = "THROTTLE",
            value = throttle,
            onValueChange = onThrottleChange,
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(modifier = Modifier.height(12.dp))

        // Volume slider
        LabeledSlider(
            label = "VOLUME",
            value = volume,
            onValueChange = onVolumeChange,
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(modifier = Modifier.height(20.dp))

        // Preset buttons
        Text("ENGINE TYPE", color = Color.Gray, fontSize = 12.sp, fontWeight = FontWeight.Bold)
        Spacer(modifier = Modifier.height(8.dp))
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            PresetButton("I4", "inline4", selectedPreset, onPresetChange, Modifier.weight(1f))
            PresetButton("V6", "v6", selectedPreset, onPresetChange, Modifier.weight(1f))
            PresetButton("V8", "v8", selectedPreset, onPresetChange, Modifier.weight(1f))
            PresetButton("V12", "v12", selectedPreset, onPresetChange, Modifier.weight(1f))
        }

        Spacer(modifier = Modifier.height(24.dp))

        // START / STOP button — big, always visible
        val btnColor = if (isRunning) Color(0xFFE53935) else Color(0xFF43A047)
        val btnText = if (isRunning) "STOP ENGINE" else "START ENGINE"

        Button(
            onClick = onToggleEngine,
            modifier = Modifier
                .fillMaxWidth()
                .height(80.dp),
            colors = ButtonDefaults.buttonColors(containerColor = btnColor),
            shape = RoundedCornerShape(12.dp)
        ) {
            Text(
                text = btnText,
                fontSize = 24.sp,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
        }

        Spacer(modifier = Modifier.height(24.dp))
    }
}

@Composable
fun RpmGauge(rpm: Float, modifier: Modifier = Modifier) {
    val maxRpm = 8000f
    val fraction = (rpm.coerceIn(0f, maxRpm)) / maxRpm

    Canvas(modifier = modifier) {
        val strokeWidth = 16.dp.toPx()
        val arcRadius = size.minDimension / 2 - strokeWidth
        val startAngle = 180f
        val sweepAngle = 180f * fraction

        // Background arc
        drawArc(
            color = Color(0xFF333333),
            startAngle = startAngle,
            sweepAngle = 180f,
            useCenter = false,
            topLeft = Offset(size.width / 2 - arcRadius, size.height / 2 - arcRadius),
            size = Size(arcRadius * 2, arcRadius * 2),
            style = Stroke(width = strokeWidth)
        )

        // Filled arc
        val gradientColor = if (fraction > 0.75f) Color(0xFFFF1744)
            else if (fraction > 0.5f) Color(0xFFFF9100)
            else Color(0xFF00E676)

        drawArc(
            color = gradientColor,
            startAngle = startAngle,
            sweepAngle = sweepAngle,
            useCenter = false,
            topLeft = Offset(size.width / 2 - arcRadius, size.height / 2 - arcRadius),
            size = Size(arcRadius * 2, arcRadius * 2),
            style = Stroke(width = strokeWidth)
        )

        // RPM text
        val rpmText = String.format("%.0f", rpm)

    }

    // Overlay RPM text using Compose Text for better rendering
    Box(
        modifier = modifier,
        contentAlignment = Alignment.BottomCenter
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text(
                text = String.format("%.0f", rpm),
                fontSize = 40.sp,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            Text(
                text = "RPM",
                fontSize = 14.sp,
                color = Color.Gray
            )
        }
    }
}

@Composable
fun StatCard(value: String, unit: String, label: String, modifier: Modifier = Modifier) {
    Card(
        modifier = modifier.height(80.dp),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2A2A2A)),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(
            modifier = Modifier.fillMaxSize().padding(12.dp),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = "$value $unit",
                fontSize = 24.sp,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            Text(
                text = label,
                fontSize = 12.sp,
                color = Color.Gray,
                fontWeight = FontWeight.Bold
            )
        }
    }
}

@Composable
fun LabeledSlider(
    label: String,
    value: Float,
    onValueChange: (Float) -> Unit,
    modifier: Modifier = Modifier
) {
    Column(modifier = modifier) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(label, color = Color.Gray, fontSize = 12.sp, fontWeight = FontWeight.Bold)
            Text(
                String.format("%.0f%%", value * 100),
                color = Color.White,
                fontSize = 12.sp,
                fontWeight = FontWeight.Bold
            )
        }
        Spacer(modifier = Modifier.height(4.dp))
        Slider(
            value = value,
            onValueChange = onValueChange,
            modifier = Modifier.fillMaxWidth(),
            colors = SliderDefaults.colors(
                thumbColor = Color.White,
                activeTrackColor = Color(0xFF00E676),
                inactiveTrackColor = Color(0xFF444444)
            )
        )
    }
}

@Composable
fun PresetButton(
    label: String,
    preset: String,
    selectedPreset: String,
    onPresetChange: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val isSelected = preset == selectedPreset
    val bgColor = if (isSelected) Color(0xFF00E676) else Color(0xFF333333)
    val textColor = if (isSelected) Color.Black else Color.White

    Button(
        onClick = { onPresetChange(preset) },
        modifier = modifier.height(48.dp),
        colors = ButtonDefaults.buttonColors(containerColor = bgColor),
        shape = RoundedCornerShape(8.dp)
    ) {
        Text(
            text = label,
            fontSize = 16.sp,
            fontWeight = FontWeight.Bold,
            color = textColor
        )
    }
}

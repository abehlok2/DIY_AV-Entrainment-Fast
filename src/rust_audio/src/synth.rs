use std::f32::consts::PI;

pub trait Synth {
    fn process(&mut self, left: &mut [f32], right: &mut [f32], sample_rate: f32);
}

pub struct BinauralBeat {
    pub amp_l: f32,
    pub amp_r: f32,
    pub base_freq: f32,
    pub beat_freq: f32,
    phase_l: f32,
    phase_r: f32,
}

impl BinauralBeat {
    pub fn new(params: &serde_json::Value) -> Self {
        Self {
            amp_l: params.get("ampL").and_then(|v| v.as_f64()).unwrap_or(0.5) as f32,
            amp_r: params.get("ampR").and_then(|v| v.as_f64()).unwrap_or(0.5) as f32,
            base_freq: params.get("baseFreq").and_then(|v| v.as_f64()).unwrap_or(200.0) as f32,
            beat_freq: params.get("beatFreq").and_then(|v| v.as_f64()).unwrap_or(4.0) as f32,
            phase_l: 0.0,
            phase_r: 0.0,
        }
    }
}

impl Synth for BinauralBeat {
    fn process(&mut self, left: &mut [f32], right: &mut [f32], sample_rate: f32) {
        let len = left.len().min(right.len());
        for i in 0..len {
            let phase_inc_l = 2.0 * PI * (self.base_freq - self.beat_freq / 2.0) / sample_rate;
            let phase_inc_r = 2.0 * PI * (self.base_freq + self.beat_freq / 2.0) / sample_rate;
            self.phase_l += phase_inc_l;
            self.phase_r += phase_inc_r;
            if self.phase_l > 2.0 * PI { self.phase_l -= 2.0 * PI; }
            if self.phase_r > 2.0 * PI { self.phase_r -= 2.0 * PI; }
            left[i] = (self.phase_l.sin()) * self.amp_l;
            right[i] = (self.phase_r.sin()) * self.amp_r;
        }
    }
}

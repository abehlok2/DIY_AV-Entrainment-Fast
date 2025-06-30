use std::f32::consts::PI;

pub fn binaural_beat(left: &mut [f32], right: &mut [f32], base_freq: f32, beat_freq: f32, sample_rate: u32) {
    let sr = sample_rate as f32;
    for i in 0..left.len() {
        let t = i as f32 / sr;
        left[i] = (2.0 * PI * (base_freq - beat_freq / 2.0) * t).sin();
        right[i] = (2.0 * PI * (base_freq + beat_freq / 2.0) * t).sin();
    }
}

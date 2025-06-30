use crate::models::Track;
use crate::synths::binaural_beat;
use rodio::{OutputStream, Sink};

pub struct RealtimeSynthContext {
    pub sample_rate: u32,
    pub buffer_size: usize,
}

impl Default for RealtimeSynthContext {
    fn default() -> Self {
        Self { sample_rate: 44100, buffer_size: 512 }
    }
}

pub struct RealtimeEngine {
    context: RealtimeSynthContext,
    track: Option<Track>,
}

impl RealtimeEngine {
    pub fn new(context: RealtimeSynthContext) -> Self {
        Self { context, track: None }
    }

    pub fn load_track(&mut self, track: Track) {
        self.track = Some(track);
    }

    pub fn play(&self) -> anyhow::Result<()> {
        let track = self.track.as_ref().ok_or_else(|| anyhow::anyhow!("no track loaded"))?;
        let (_stream, stream_handle) = OutputStream::try_default()?;
        let sink = Sink::try_new(&stream_handle)?;

        for step in &track.steps {
            let num_samples = (step.duration * self.context.sample_rate as f32) as usize;
            let mut left = vec![0.0f32; num_samples];
            let mut right = vec![0.0f32; num_samples];
            for voice in &step.voices {
                match voice.synth_function_name.as_str() {
                    "binaural_beat" => {
                        if let (Some(base), Some(beat)) = (
                            voice.params.get("baseFreq").and_then(|v| v.as_f64()),
                            voice.params.get("beatFreq").and_then(|v| v.as_f64()),
                        ) {
                            binaural_beat(&mut left, &mut right, base as f32, beat as f32, self.context.sample_rate);
                        }
                    }
                    _ => {}
                }
            }
            let samples: Vec<f32> = left.iter().zip(right.iter()).flat_map(|(&l, &r)| [l, r]).collect();
            sink.append(rodio::buffer::SamplesBuffer::new(2, self.context.sample_rate, samples));
        }
        sink.sleep_until_end();
        Ok(())
    }
}

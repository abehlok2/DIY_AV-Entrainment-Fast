use crate::models::*;
use crate::synth::{BinauralBeat, Synth};
use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};

pub struct RealtimeEngine {
    track: Track,
    current_step: usize,
    synths: Vec<Box<dyn Synth + Send>>,
}

impl RealtimeEngine {
    pub fn new(track: Track) -> Self {
        Self { track, current_step: 0, synths: Vec::new() }
    }

    pub fn prepare(&mut self) {
        if let Some(step) = self.track.steps.get(0) {
            self.synths.clear();
            for voice in &step.voices {
                if voice.synth_function == "binaural_beat" {
                    self.synths.push(Box::new(BinauralBeat::new(&voice.params)));
                }
            }
        }
    }

    pub fn play(&mut self) -> anyhow::Result<()> {
        let host = cpal::default_host();
        let device = host.default_output_device().ok_or_else(|| anyhow::anyhow!("no output device"))?;
        let config = device.default_output_config()?;
        let sample_rate = self.track.global_settings.sample_rate as f32;
        let mut synths = std::mem::take(&mut self.synths);

        let channels = config.channels() as usize;
        let err_fn = |err| eprintln!("stream error: {}", err);
        let stream = match config.sample_format() {
            cpal::SampleFormat::F32 => device.build_output_stream(&config.into(), move |data: &mut [f32], _| {
                let frames = data.len() / channels;
                let mut left = vec![0.0f32; frames];
                let mut right = vec![0.0f32; frames];
                for synth in &mut synths {
                    synth.process(&mut left, &mut right, sample_rate);
                }
                for i in 0..frames {
                    data[i * channels] = left[i];
                    if channels > 1 {
                        data[i * channels + 1] = right[i];
                    }
                }
            }, err_fn, None),
            _ => unimplemented!(),
        }?;
        stream.play()?;
        std::thread::sleep(std::time::Duration::from_secs_f32(1.0));
        Ok(())
    }
}

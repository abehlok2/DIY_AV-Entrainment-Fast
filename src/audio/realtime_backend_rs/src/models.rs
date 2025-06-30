use serde::{Serialize, Deserialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Voice {
    pub synth_function_name: String,
    #[serde(default)]
    pub is_transition: bool,
    #[serde(default)]
    pub params: serde_json::Value,
    #[serde(default)]
    pub description: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Step {
    pub duration: f32,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub voices: Vec<Voice>,
}

#[derive(Debug, Serialize, Deserialize, Clone, Default)]
pub struct GlobalSettings {
    #[serde(default = "default_sample_rate")]
    pub sample_rate: u32,
    #[serde(default = "default_crossfade_duration")]
    pub crossfade_duration: f32,
    #[serde(default = "default_crossfade_curve")]
    pub crossfade_curve: String,
    #[serde(default)]
    pub output_filename: Option<String>,
}

fn default_sample_rate() -> u32 { 44100 }
fn default_crossfade_duration() -> f32 { 1.0 }
fn default_crossfade_curve() -> String { "linear".into() }

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Track {
    #[serde(default)]
    pub global_settings: GlobalSettings,
    #[serde(default)]
    pub steps: Vec<Step>,
}

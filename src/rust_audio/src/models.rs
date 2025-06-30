use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Voice {
    #[serde(rename = "synth_function_name")]
    pub synth_function: String,
    #[serde(default)]
    pub params: serde_json::Value,
    #[serde(default, rename = "is_transition")]
    pub is_transition: bool,
    #[serde(default)]
    pub description: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Step {
    #[serde(rename = "duration")]
    pub duration_seconds: f32,
    #[serde(default)]
    pub voices: Vec<Voice>,
    #[serde(default)]
    pub description: String,
}

#[derive(Debug, Serialize, Deserialize, Clone, Default)]
pub struct GlobalSettings {
    #[serde(default = "default_sample_rate", rename = "sample_rate")]
    pub sample_rate: f32,
}

fn default_sample_rate() -> f32 { 44100.0 }

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Track {
    #[serde(default)]
    pub global_settings: GlobalSettings,
    #[serde(default)]
    pub steps: Vec<Step>,
}

pub fn load_track(path: &std::path::Path) -> std::io::Result<Track> {
    let text = std::fs::read_to_string(path)?;
    let track: Track = serde_json::from_str(&text).map_err(|e| {
        std::io::Error::new(std::io::ErrorKind::InvalidData, e.to_string())
    })?;
    Ok(track)
}

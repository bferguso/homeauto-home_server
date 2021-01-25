alter table public.ha_remote_devices
    add COLUMN node_capabilities_json jsonb default '{}'::jsonb;
alter table public.ha_remote_devices
    rename COLUMN sensor_location to node_location;
alter table public.ha_remote_devices
    rename COLUMN device_active to node_active;

alter table public.ha_environment_log
    rename COLUMN sensor_location to node_location;

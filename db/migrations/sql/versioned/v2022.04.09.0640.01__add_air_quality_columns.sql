--liquibase formatted sql
--changeset qed:v2022.04.09.0640.01__add_air_quality_columns splitStatements:true endDelimiter:;
alter table ha_environment_log
    add COLUMN voc numeric(4,0);

alter table ha_environment_log
    add COLUMN co2 numeric(4,0);

--rollback alter table ha_environment_log drop column co2;
--rollback alter table ha_environment_log drop column voc;

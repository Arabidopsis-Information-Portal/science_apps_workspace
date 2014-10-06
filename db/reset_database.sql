--
-- Resets the database from all science_app_workspace changes.
--
-- WARNING: THIS SCRIPT WILL DELETE NODE DATA! USE WITH CAUTION!
--

drop table field_revision_science_app_repository;
drop table field_revision_science_app_version;
drop table field_data_science_app_repository;
drop table field_data_science_app_version;

delete from field_config_instance where bundle = 'science_app';
delete from field_config where field_name in ('science_app_repository','science_app_version');

delete from node_revision where nid in (select * from node where type = 'science_app');
delete from node where type = 'science_app';
delete from node_type where type = 'science_app';
